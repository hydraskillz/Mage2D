#include "MageLib.h"

using namespace mage;

//---------------------------------------
FileLoadJob::FileLoadJob( const std::string& filename,
	const std::string& onCompleteEventName,
	JobPriority priority )
	: Job( priority, Job::JOB_FILE_IO )
	, mFilename( filename )
	, mEventName( onCompleteEventName )
	, mLoadedData( NULL )
	, mLoadedDataSize( 0U )
	, mLoadStatusCode( FSE_NO_ERROR )
{}
//---------------------------------------
FileLoadJob::~FileLoadJob()
{ /* We do not delete the loaded data here - it is owned by the caller. */ }
//---------------------------------------
void FileLoadJob::OnExecute()
{
	mLoadStatusCode = OpenDataFile( mFilename.c_str(), mLoadedData, mLoadedDataSize );
}
//---------------------------------------
void FileLoadJob::OnCompletetion()
{
	Dictionary params;

	params.Set( "FileName", mFilename );
	params.Set( "FileLoadCode", mLoadStatusCode );
	params.Set( "FileLoadSize", mLoadedDataSize );
	params.Set( "FileLoadData", mLoadedData );

	EventManager::FireEvent( mEventName, params );
}
//---------------------------------------