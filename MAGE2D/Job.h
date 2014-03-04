/*
 * Author      : Matthew Johnson
 * Date        : 11/Jun/2013
 * Description :
 *   ABC for Jobs
 */
 
#pragma once

namespace mage
{

	class Job
	{
	public:
		enum JobPriority
		{
			JP_LOWEST,
			JP_LOW,
			JP_BELOW_AVERAGE,
			JP_AVERAGE,
			JP_ABOVE_AVERAGE,
			JP_HIGH,
			JP_HIGHEST
		};

		enum JobType
		{
			JOB_GENERIC,
			JOB_FILE_IO,
			JOB_TYPE_COUNT
		};

		Job( JobPriority priority=JP_AVERAGE, JobType jobType=JOB_GENERIC );
		virtual ~Job();

		virtual void OnExecute() = 0;
		virtual void OnCompletetion() = 0;

		JobType     GetJobType()     const { return mJobType;  }
		JobPriority GetJobPriority() const { return mPriority; }

		bool operator< ( const Job& other ) const
		{
			return mPriority < other.mPriority;
		}

	protected:
		JobPriority mPriority;
		JobType mJobType;
	};

}