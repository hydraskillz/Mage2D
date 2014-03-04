/*
 * Author      : Matthew Johnson
 * Date        : 11/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class Entity;

	class TriggeredEvent
	{
	public:
		TriggeredEvent();
		~TriggeredEvent();

		void AddEvent( const Dictionary& event );
		// Returns false if TriggerCount == 0 after the events are fired
		bool FireEvents( Entity* invoker, Entity* instigator=NULL );
		void SetTriggerCount( int count );
		void SetTriggerFlags( const std::string& flags );

	private:
		std::vector< Dictionary > mEventList;
		int mTriggerCount;
		std::string mTriggerFlags;
	};

}