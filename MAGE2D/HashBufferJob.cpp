#include "MageLib.h"

using namespace mage;

//---------------------------------------
HashBufferJob::HashBufferJob( const char* buffer,
	const std::string& onCompletedEvenName,
	Job::JobPriority priority )
	: Job( priority, Job::JOB_GENERIC )
	, mBuffer( buffer )
	, mEventName( onCompletedEvenName )
	, mHash( 0 )
{}
//---------------------------------------
HashBufferJob::~HashBufferJob()
{}
//---------------------------------------
void HashBufferJob::OnExecute()
{
	//mHash = GenerateHash( mBuffer );
}
//---------------------------------------
void HashBufferJob::OnCompletetion()
{
	Dictionary params;

	params.Set( "Buffer", mBuffer );
	params.Set( "Hash", mHash );

	EventManager::FireEvent( mEventName, params );
}
//---------------------------------------