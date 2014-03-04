#include "MageLib.h"

using namespace mage;

//---------------------------------------
JobManager* JobManager::msJobManager;
//---------------------------------------
void JobManager::CreateJobManager()
{
	msJobManager = new JobManager;
	msJobManager->Initialize();
}
//---------------------------------------
JobManager* JobManager::GetInstance()
{
	assertion( msJobManager != NULL, "JobManager not created!\nCall CreateJobManager() first." );
	return msJobManager;
}
//---------------------------------------
void JobManager::DestroyJobManager()
{
	msJobManager->DestroyAllWorkers();

	// If we REALLY want to clean up memory before exiting we would delete the JobManager.
	// But then we need to make sure all the works are done with jobs (and dead) to avoid cross thread
	// access violations.
	//Delete0( msJobManager );
}
//---------------------------------------

//---------------------------------------
// Worker Thread
void JobManager::_worker_function( void* pWorker )
{
	Worker* worker = (Worker*) pWorker;
	JobManager* jobManager = JobManager::GetInstance();

	while ( worker->Alive )
	{
		// Try to get a job
		worker->CurrentJob = jobManager->AquireNextJob( worker->MyPerferedJobType );
		
		// Wait a bit before trying to get another job
		if ( !worker->CurrentJob )
		{
			Thread::Sleep( 100 );
		}
		else
		{
			worker->CurrentJob->OnExecute();
			jobManager->SubmitCompletedJob( worker->CurrentJob );
		}
	}

	delete worker;
}
//---------------------------------------


//---------------------------------------
JobManager::JobManager()
{
	mMaxNumberOfWorkers = 0;//Thread::GetMaxThreadConcurrency();

	if ( mMaxNumberOfWorkers == 0 )
	{
		ConsolePrintf( CONSOLE_INFO, "JobManager : System does not support multiple concurrent threads.\n" );
	}
	else
	{
		ConsolePrintf( CONSOLE_INFO, "JobManager : Max concurrent threads is %d.\n", mMaxNumberOfWorkers );
	}
}
//---------------------------------------
JobManager::~JobManager()
{
	DestroyAllWorkers();
}
//---------------------------------------
void JobManager::Initialize()
{
	if ( mMaxNumberOfWorkers > 0 )
	{
		for ( unsigned int i = 0; i < mMaxNumberOfWorkers; ++i )
		{
			Worker* worker = new Worker;
			worker->MyThread = new Thread( _worker_function, (void*) worker );
			mWorkers.push_back( worker );
		}

		// Set the last work to do file I/O for now...
		mWorkers.back()->MyPerferedJobType = Job::JOB_FILE_IO;
	}
}
//---------------------------------------
void JobManager::OnUpdate()
{
	// If there are no workers to do jobs, do them ourself
	if ( mMaxNumberOfWorkers == 0 )
	{
		Job* job = NULL;

		// Try to distribute work across all types evenly
		static int nextType = 0;
		++nextType;
		if ( nextType >= Job::JOB_TYPE_COUNT ) nextType = 0;

		if ( !mPendingJobQueue[ nextType ].empty() )
		{
			job = mPendingJobQueue[ nextType ].back();
			mPendingJobQueue[ nextType ].pop_back();
		}
		// There was not job of the next 'even' type, look for another job of any type
		else
		{
			for ( int i = 0; i < Job::JOB_TYPE_COUNT; ++i )
			{
				if ( !mPendingJobQueue[ i ].empty() )
				{
					job = mPendingJobQueue[ i ].back();
					mPendingJobQueue[ i ].pop_back();
					break;
				}
			}
		}

		// Execute job if there is one
		if ( job )
		{
			job->OnExecute();
			SubmitCompletedJob( job );
		}
	}

	// Fire callbacks for completed jobs
	std::list< Job* > completedJobs;

	BeginCriticalSection( mJobCompleteMutex );
	
	completedJobs = mCompletedJobList;
	mCompletedJobList.clear();
	
	EndCriticalSection();

	for ( auto jobItr = completedJobs.begin(); jobItr != completedJobs.end(); ++ jobItr )
	{
		(*jobItr)->OnCompletetion();
		delete *jobItr;
	}
}
//---------------------------------------
void JobManager::PushJob( Job* job )
{
	CriticalBlock( mJobQueueMutex );

	// Add new job
	mPendingJobQueue[ job->GetJobType() ].push_front( job );
	// Sort on priority
	mPendingJobQueue[ job->GetJobType() ].sort( [&]( const Job* jobA, const Job* jobB ) -> bool
	{
		return *jobA < *jobB;
	});
}
//---------------------------------------
void JobManager::DestroyAllWorkers()
{
	for ( auto itr = mWorkers.begin(); itr != mWorkers.end(); ++itr )
	{
		(*itr)->Alive = false;
	}
	mWorkers.clear();
}
//---------------------------------------
void JobManager::DestroyAllPendingJobs()
{
	CriticalBlock( mJobQueueMutex );

	for ( int i = 0; i < Job::JOB_TYPE_COUNT; ++i )
	{
		while ( !mPendingJobQueue[ i ].empty() )
		{
			delete mPendingJobQueue[ i ].back();
			mPendingJobQueue[ i ].pop_back();
		}
	}
}
//---------------------------------------
void JobManager::DestroyAllPendingJobsOfType( Job::JobType jobType )
{
	CriticalBlock( mJobQueueMutex );

	while ( !mPendingJobQueue[ jobType ].empty() )
	{
		delete mPendingJobQueue[ jobType ].back();
		mPendingJobQueue[ jobType ].pop_back();
	}
}
//---------------------------------------
void JobManager::SetMaxWorkerThreads( int maxWorkers )
{
	mMaxNumberOfWorkers = maxWorkers;

	// Limit number of workers
	if ( mWorkers.size() > mMaxNumberOfWorkers )
	{
		for ( unsigned int i = mMaxNumberOfWorkers; i < mWorkers.size(); ++i )
		{
			mWorkers[ i ]->Alive = false;
		}
		// Remove dead workers from the list
		// Dead workers will free there own memory when their thread dies
		mWorkers.erase( std::remove_if( mWorkers.begin(), mWorkers.end(), [&]( const Worker* worker ) -> bool
		{
			return worker->Alive;
		}), mWorkers.end() );
	}
	// Make more workers
	else if ( mWorkers.size() < mMaxNumberOfWorkers )
	{
		for ( unsigned int i = mWorkers.size(); i < mMaxNumberOfWorkers; ++i )
		{
			Worker* worker = new Worker;
			worker->MyThread = new Thread( _worker_function, (void*) worker );
			mWorkers.push_back( worker );
		}
		// Hack to make sure a file IO worker is always around
		// @TODO generic workers should take over file IO if no file IO work is available.
		bool canLoadFiles = false;
		for ( unsigned int i = mWorkers.size(); i < mMaxNumberOfWorkers; ++i )
		{
			if ( mWorkers[i]->MyPerferedJobType == Job::JOB_FILE_IO )
			{
				canLoadFiles = true;
				break;
			}
		}
		if ( !canLoadFiles )
		{
			mWorkers.back()->MyPerferedJobType = Job::JOB_FILE_IO;
		}
	}
}
//---------------------------------------
int JobManager::GetCurrentNumberOfWorkers() const
{
	return mMaxNumberOfWorkers;
}
//---------------------------------------
Job* JobManager::AquireNextJob( Job::JobType requestedType )
{
	Job* job = NULL;

	BeginCriticalSection( mJobQueueMutex );

	// Try to get the requested type of job
	if ( !mPendingJobQueue[ requestedType ].empty() )
	{
		job = mPendingJobQueue[ requestedType ].back();
		mPendingJobQueue[ requestedType ].pop_back();
		//ConsolePrintf( CONSOLE_INFO, "Job %d started!\n", requestedType );
	}
	// We have no jobs of that type, look for other jobs
	// @TODO this will cause generic works to load files, which is generally not wanted.
	/*else
	{
		for ( int i = 0; i < Job::JOB_TYPE_COUNT; ++i )
		{
			if ( !mPendingJobQueue[ i ].empty() )
			{
				job = mPendingJobQueue[ i ].back();
				mPendingJobQueue[ i ].pop_back();
				break;
			}
		}
	}*/

	EndCriticalSection();

	return job;
}
//---------------------------------------
void JobManager::SubmitCompletedJob( Job* job )
{
	CriticalBlock( mJobCompleteMutex );
	
	mCompletedJobList.push_back( job );
}
//---------------------------------------