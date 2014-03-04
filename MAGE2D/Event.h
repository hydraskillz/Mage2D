/*
 * Author      : Matthew Johnson
 * Date        : 19/Apr/2013
 * Description :
 *   Event callback system.
 */
 
#pragma once

namespace mage
{
	//---------------------------------------
	// Static class for registering and firing events.
	// Derive from EventListener to have events auto unregistered for a class.
	// (or call UnregisterObjectForAllEvent( *this ) in your destructor)
	//---------------------------------------
	class EventManager
	{
	private:
		//---------------------------------------
		// Base for event callback wrapper
		//---------------------------------------
		class EventCallbackBase
		{
		protected:	// Abstract
			EventCallbackBase() {}
		public:
			virtual ~EventCallbackBase() {}

			virtual void Execute( Dictionary& params ) const = 0;
		};
		//---------------------------------------


		//---------------------------------------
		// Event callback wrapper
		//---------------------------------------
		template< typename T >
		class EventCallback
			: public EventCallbackBase
		{
		public:
			//private:
			typedef void( T::*CBFunc )( Dictionary& );
			T* mObject;
			CBFunc mCB;

		public:
			EventCallback( T* object, CBFunc func )
				: mObject( object )
				, mCB( func )
			{}
			virtual ~EventCallback() {}

			virtual void Execute( Dictionary& params ) const
			{
				(mObject->*mCB)(params);
			}
		};
		//---------------------------------------

	public:
		typedef std::map< std::string, std::vector< EventCallbackBase* > > EventCallbackMap;
		typedef std::vector< EventCallbackBase* > EventCallbackList;

	private:
		EventManager();

		//---------------------------------------
		static EventCallbackMap EventCallbacks;
		static Dictionary EMPTY_PARAMS;
		//---------------------------------------

	public:
		~EventManager();

		//---------------------------------------
		static int FireEvent( const std::string& eventName, Dictionary& params )
		{
			EventCallbackList& callbacks = EventCallbacks[ eventName ];

			for ( EventCallbackList::iterator event = callbacks.begin(); event != callbacks.end(); ++event )
			{
				(*event)->Execute( params );
			}

			return static_cast< int >( callbacks.size() );
		}
		//---------------------------------------
		template< typename T, typename TFUNC >
		static void RegisterObjectForEvent( const std::string& eventName, T& object, TFUNC func )
		{
			EventCallbackList& callbacks = EventCallbacks[ eventName ];
			callbacks.push_back( new EventCallback< T >( &object, func ) );
		}
		//---------------------------------------
		static int FireEvent( const std::string& eventName )
		{
			return FireEvent( eventName, EMPTY_PARAMS );
		}
		//---------------------------------------
		template< typename T, typename TFUNC >
		static void UnregisterObjectForEvent( const std::string& eventName, T& object, TFUNC func )
		{
			EventCallbackList& callbacks = EventCallbacks[ eventName ];
			callbacks.erase(
				std::remove_if( callbacks.begin(), callbacks.end(),
				[&]( EventCallbackBase*& cb ) -> bool
			{
				EventManager::EventCallback< T >* c = dynamic_cast< EventManager::EventCallback< T >* >( cb );
				if ( c && c->mObject == &object && c->mCB == func )
				{
					delete cb;
					return true;
				}
				return false;
			}),
				callbacks.end() );
		}
		//---------------------------------------
		template< typename T >
		static void UnregisterObjectForEvent( const std::string& eventName, T& object )
		{
			EventCallbackList& callbacks = EventCallbacks[ eventName ];
			callbacks.erase(
				std::remove_if( callbacks.begin(), callbacks.end(),
				[&]( EventCallbackBase*& cb ) -> bool
			{
				EventManager::EventCallback< T >* c = dynamic_cast< EventManager::EventCallback< T >* >( cb );
				if ( c && c->mObject == &object )
				{
					delete cb;
					return true;
				}
				return false;
			}),
				callbacks.end() );
		}
		//---------------------------------------
		template< typename T >
		static void UnregisterObjectForAllEvent( T& object )
		{
			for ( EventCallbackMap::iterator event = EventCallbacks.begin();
				event != EventCallbacks.end(); ++event )
			{
				UnregisterObjectForEvent( event->first, object );
			}
		}
		//---------------------------------------
	};
	//---------------------------------------
}

