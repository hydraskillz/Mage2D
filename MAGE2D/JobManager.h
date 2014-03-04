/*
 * Author      : Matthew Johnson
 * Date        : 11/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class JobManager
	{
		JobManager();
		~JobManager();
	public:
		static void CreateJobManager();
		static JobManager* GetInstance();
		static void DestroyJobManager();
		
		// Update sends out signals for completed jobs
		void OnUpdate();
		// Add a job to the job queue
		void PushJob( Job* job );
		// Destroy all jobs of all types
		void DestroyAllPendingJobs();
		// Destroy all jobs of a given type
		void DestroyAllPendingJobsOfType( Job::JobType jobType );
		// Set the maximum number of workers allowed
		// This value defaults to the maximum amount of threads that can be executed concurrently on the system.
		void SetMaxWorkerThreads( int maxWorkers );
		// Get the current number of worker threads
		int GetCurrentNumberOfWorkers() const;
		
	private:
		static void _worker_function( void* pWorker );
		void Initialize();
		Job* AquireNextJob( Job::JobType requestedType );
		void SubmitCompletedJob( Job* job );
		void DestroyAllWorkers();

		unsigned int mMaxNumberOfWorkers;

		struct Worker
		{
			Worker()
				: CurrentJob( NULL)
				, Alive( true )
				, MyPerferedJobType( Job::JOB_GENERIC )
			{}

			Job* CurrentJob;
			Thread* MyThread;
			bool Alive;
			Job::JobType MyPerferedJobType;
		};

		std::vector< Worker* > mWorkers;
		// New jobs are added to the front, works acquire jobs from the back
		std::list< Job* > mPendingJobQueue[ Job::JOB_TYPE_COUNT ];
		std::list< Job* > mCompletedJobList;

		Mutex mJobQueueMutex;
		Mutex mJobCompleteMutex;

		static JobManager* msJobManager;

	};

}