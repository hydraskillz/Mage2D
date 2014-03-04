/*
 * Author      : Matthew Johnson
 * Date        : 26/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameTrigger;
	class GameTriggerAlways;


	//---------------------------------------
	// GameTriggerManager
	//---------------------------------------
	class GameTriggerManager
	{
	public:
		static void CreateTrigger( const std::string& ownerName,
			const XmlReader::XmlReaderIterator& trigItr );
		static GameTrigger* NewTrigger( const std::string& name, GameObject* owner );
		static void DestroyAllTriggers();

	private:
		static std::map< std::string, GameTrigger* > mGameTriggers;
	};
	//---------------------------------------


	//---------------------------------------
	// GameTrigger
	//---------------------------------------
	class GameTrigger
		: public GameLogicNode
	{
	public:

		enum GameTriggerType
		{
			GT_ALWAYS,
			GT_EVENT,
			GT_COLLISION,
			GT_PROPERTY,
			GT_ANIMATION,
			GT_TYPE_COUNT
		};

		enum GameTriggerState
		{
			GT_INACTIVE,
			GT_JUST_ACTIVATED,
			GT_ACTIVATED,
			GT_JUST_DEACTIVATED
		};

	protected:
		GameTrigger( const std::string& name,
			const XmlReader::XmlReaderIterator& trigItr );
		//GameTrigger( const GameTrigger& other );
	public:
		virtual ~GameTrigger();
		virtual void OnUpdate( float dt );
		virtual void OnCreate() {}
		virtual GameTrigger* Copy() const = 0;
		virtual void Reset();

		inline bool IsActive()
		{
			if ( mRepeatOk )
			{
				if ( !mRepeatTrueSignal || mOnce )
				{
					return mIsActive == GT_JUST_ACTIVATED;
				}
				return ( mIsActive == GT_JUST_ACTIVATED || mIsActive == GT_ACTIVATED );
			}
			return false;
		}
		
	protected:
		GameTriggerType mType;
		GameTriggerState mIsActive;
		uint32 mTickDelay;
		uint32 mLastTickFrame;
		bool mRepeatTrueSignal;
		bool mRepeatFalseSignal;
		bool mOnce;
		bool mInvertSignal;
		bool mRepeatOk;
	};
	//---------------------------------------


	//---------------------------------------
	// GameTriggerAlways
	//---------------------------------------
	class GameTriggerAlways
		: public GameTrigger
	{
		friend class GameTriggerManager;
	protected:
		GameTriggerAlways( const std::string& name,
			const XmlReader::XmlReaderIterator& trigItr );
		//GameTriggerAlways( const GameTriggerAlways& other );
	public:
		virtual ~GameTriggerAlways();
		virtual void OnUpdate( float dt );
		virtual GameTrigger* Copy() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameTriggerEvent
	//---------------------------------------
	class GameTriggerEvent
		: public GameTrigger
	{
		friend class GameTriggerManager;
	protected:
		GameTriggerEvent( const std::string& name,
			const XmlReader::XmlReaderIterator& trigItr );
		//GameTriggerEvent( const GameTriggerEvent& other );
	public:
		virtual ~GameTriggerEvent();
		virtual void OnUpdate( float dt );
		virtual void OnCreate();
		virtual GameTrigger* Copy() const;
		virtual void Reset();

		void EventReceived( Dictionary& params );
	
		std::string EventName;

	protected:
		bool mRecieved;
		std::string mRegisteredEvent;
	};
	//---------------------------------------


	//---------------------------------------
	// GameTriggerCollision
	//---------------------------------------
	class GameTriggerCollision
		: public GameTrigger
	{
		friend class GameTriggerManager;
	protected:
		GameTriggerCollision( const std::string& name,
			const XmlReader::XmlReaderIterator& trigItr );
	public:
		virtual ~GameTriggerCollision();
		virtual void OnUpdate( float dt );
		virtual GameTrigger* Copy() const;

	protected:
		std::string mProperty;
	};
	//---------------------------------------


	//---------------------------------------
	// GameTriggerProperty
	//---------------------------------------
	class GameTriggerProperty
		: public GameTrigger
	{
		friend class GameTriggerManager;
	protected:
		GameTriggerProperty( const std::string& name,
			const XmlReader::XmlReaderIterator& trigItr );
	public:
		virtual ~GameTriggerProperty();
		virtual void OnUpdate( float dt );
		virtual void OnCreate();
		virtual GameTrigger* Copy() const;

		enum PropertyComparisonType
		{
			GT_PROP_EQUAL,
			GT_PROP_NOT_EQUAL,
			GT_PROP_CHANGED,
			GT_PROP_INTERVAL
		};

	protected:
		std::string mProperty;
		std::string mValue, mValue2;
		PropertyComparisonType mCompareType;
		bool mBadProperty;
	};
	//---------------------------------------


	//---------------------------------------
	// GameTriggerAnimation
	//---------------------------------------
	class GameTriggerAnimation
		: public GameTrigger
	{
		friend class GameTriggerManager;
	protected:
		GameTriggerAnimation( const std::string& name,
			const XmlReader::XmlReaderIterator& trigItr );
	public:
		virtual ~GameTriggerAnimation();
		virtual void OnUpdate( float dt );
		virtual GameTrigger* Copy() const;

		enum AnimCheckMode
		{
			GA_ANIM_COMPLETE
		};

	protected:
		std::string mAnimName;
		AnimCheckMode mCompareType;
	};
	//---------------------------------------
}