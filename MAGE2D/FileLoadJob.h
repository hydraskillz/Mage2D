/*
 * Author      : Matthew Johnson
 * Date        : 16/Jun/2013
 * Description :
 *   Load a data file into a buffer.
 *   On completion, an event is fired (caller specified name).
 *   The event contains the following:
 *   - string FileName       = the name of the file that was loaded (same as filename arg).
 *   - int    FileLoadCode   = return value from the filesystem call.
 *   - uint   FileLoadSize   = size of the data buffer in bytes. This is 0 if an error occurred.
 *   - char*  FileLoadData   = the data that was loaded. This is NULL if an error occurred.
 *   The caller takes ownership of the data pointer.
 */
 
#pragma once

namespace mage
{

	class FileLoadJob
		: public Job
	{
	public:
		FileLoadJob( const std::string& filename,
					 const std::string& onCompleteEventName,
					 JobPriority priority=Job::JP_AVERAGE );
		virtual ~FileLoadJob();

		virtual void OnExecute();
		virtual void OnCompletetion();

	private:
		std::string mFilename;
		std::string mEventName;
		char* mLoadedData;
		unsigned int mLoadedDataSize;
		int mLoadStatusCode;
	};

}