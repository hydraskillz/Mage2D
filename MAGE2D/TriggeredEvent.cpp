#include "MageLib.h"

using namespace mage;

//---------------------------------------
TriggeredEvent::TriggeredEvent()
	: mTriggerCount( 0 )
{}
//---------------------------------------
TriggeredEvent::~TriggeredEvent()
{}
//---------------------------------------
void TriggeredEvent::AddEvent( const Dictionary& event )
{
	mEventList.push_back( event );
}
//---------------------------------------
bool TriggeredEvent::FireEvents( Entity* invoker, Entity* instigator )
{
	if ( mTriggerCount < 0 ) return false;

	if ( mTriggerFlags.length() &&
		 instigator && 
		 mTriggerFlags != instigator->GetUserFlags() ) return false;

	for ( auto itr = mEventList.begin(); itr != mEventList.end(); ++itr )
	{
		Dictionary params;
		std::string eventName;
		itr->Get( "Args", params );
		itr->Get( "EventName", eventName );
		params.Set( "Invoker", invoker );
		params.Set( "Instigator", instigator );
		EventManager::FireEvent( eventName, params );	
	}

	if ( mTriggerCount > 0 )
	{
		--mTriggerCount;
		if ( mTriggerCount == 0 )
		{
			mTriggerCount = -1;
			return false;
		}
	}
	return true;
}
//---------------------------------------
void TriggeredEvent::SetTriggerCount( int count )
{
	mTriggerCount = count;
}
//---------------------------------------
void TriggeredEvent::SetTriggerFlags( const std::string& flags )
{
	mTriggerFlags = flags;
}
//---------------------------------------