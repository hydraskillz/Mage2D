/*
 * Author      : Matthew Johnson
 * Date        : 16/Jun/2013
 * Description :
 *   Generate a hash for a byte buffer.
 *   On completion, an event is fired (caller specified name).
 *   The event contains the following:
 *   - char*  Buffer  = the buffer that was passed in
 *   - uint   Hash    = the computed hash
 */
 
#pragma once

namespace mage
{

	class HashBufferJob
		: public Job
	{
	public:
		HashBufferJob( const char* buffer,
					   const std::string& onCompletedEvenName,
					   Job::JobPriority priority=Job::JP_AVERAGE );
		virtual ~HashBufferJob();

		virtual void OnExecute();
		virtual void OnCompletetion();

	private:
		const char* mBuffer;
		std::string mEventName;
		unsigned int mHash;
	};

}