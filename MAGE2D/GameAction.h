/*
 * Author      : Matthew Johnson
 * Date        : 26/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameAction;
	class GameActionProperty;

	//---------------------------------------
	// GameActionManager
	//---------------------------------------
	class GameActionManager
	{
	public:
		static void CreateAction( const std::string& ownerName,
			const XmlReader::XmlReaderIterator& actItr );
		static GameAction* NewAction( const std::string& name, GameObject* owner );
		static void DestroyAction( const GameTrigger& trigger );
		static void DestroyAllActions();

	private:
		static std::map< std::string, GameAction* > mGameActions;
	};
	//---------------------------------------


	//---------------------------------------
	// GameAction
	//---------------------------------------
	class GameAction
		: public GameLogicNode
	{
	public:

		enum GameActionType
		{
			GA_PROPERTY,
			GA_ADDOBJECT,
			GA_MOTION,
			GA_LOADLEVEL,
			GA_SEND_EVENT,
			GA_STATE,
			GA_REMOVEOBJECT,
			GA_RESTART_LEVEL,
			GA_CAMERA,
			GA_ANIMATION,
			GA_PHYSICS,
			GA_SOUND,
			GA_TYPE_COUNT
		};

		virtual ~GameAction();
	protected:
		GameAction( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
		//GameAction( const GameAction& other );
	public:

		virtual void OnActivate() = 0;
		virtual void OnDeactivate() {}
		virtual void OnCreate() {}
		virtual GameAction* Copy() const = 0;

	protected:
		GameActionType mType;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionProperty
	//---------------------------------------
	class GameActionProperty
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionProperty( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
		//GameActionProperty( const GameActionProperty& other );
	public:
		virtual ~GameActionProperty();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

	protected:
		enum PropertyManipulationType
		{
			PROPMANIP_NULL,
			PROPMANIP_ADD,
			PROPMANIP_ASSIGN
		};

		std::string mPropertyName;
		std::string mValue;
		PropertyManipulationType mPropManipType;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionAddObject
	//---------------------------------------
	class GameActionAddObject
		: public GameAction
	{
			friend class GameActionManager;
		protected:
			GameActionAddObject( const std::string& name,
				const XmlReader::XmlReaderIterator& actItr );
		public:
			virtual ~GameActionAddObject();

			virtual void OnActivate();
			virtual GameAction* Copy() const;

		protected:
			std::string mObjectToAdd;
			Vec2f mSpawnOffset;
			Vec2f mSpawnVelocity;
			bool mAttachToParent;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionMotion
	//---------------------------------------
	class GameActionMotion
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionMotion( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionMotion();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

	protected:
		enum MotionType
		{
			MOTION_MOVE,
			MOTION_FORCE,
			MOTION_VELOCITY
		};
		MotionType mMotionType;
		Vec2f mMotion[3];
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionLoadLevel
	//---------------------------------------
	class GameActionLoadLevel
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionLoadLevel( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionLoadLevel();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

	protected:
		std::string mLevelName;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionSendEvent
	//---------------------------------------
	class GameActionSendEvent
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionSendEvent( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionSendEvent();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

	protected:
		std::string mEventName;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionState
	//---------------------------------------
	class GameActionState
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionState( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionState();

		virtual void OnActivate();
		virtual void OnCreate();
		virtual GameAction* Copy() const;

		enum StateOperationMode
		{
			STATE_OP_SET,
			STATE_OP_ADD,
			STATE_OP_SUB,
			STATE_OP_INV
		};

	protected:
		StateOperationMode mOpMode;
		uint32 mStateMask;
		std::vector< std::string > mStateNames;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionRemoveObject
	//---------------------------------------
	class GameActionRemoveObject
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionRemoveObject( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionRemoveObject();

		virtual void OnActivate();
		virtual GameAction* Copy() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionRestartLevel
	//---------------------------------------
	class GameActionRestartLevel
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionRestartLevel( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionRestartLevel();

		virtual void OnActivate();
		virtual GameAction* Copy() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionCamera
	//---------------------------------------
	class GameActionCamera
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionCamera( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionCamera();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

	protected:
		std::string mObjectToLookAt;
		RectI mCameraBounds;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionAnimation
	//---------------------------------------
	class GameActionAnimation
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionAnimation( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionAnimation();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

	protected:
		std::string mAnimName;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionPhysics
	//---------------------------------------
	class GameActionPhysics
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionPhysics( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionPhysics();

		virtual void OnActivate();
		virtual GameAction* Copy() const;

		enum PhysicsMode
		{
			GA_SUSPEND_DYNAMICS,
			GA_RESTORE_DYNAMICS,
			GA_SET_MASS,
			GA_SET_GRAVITY
		};

	protected:
		PhysicsMode mMode;
		uint32 mMass;
		Vec2f mGravity;
	};
	//---------------------------------------


	//---------------------------------------
	// GameActionSound
	//---------------------------------------
	class GameActionSound
		: public GameAction
	{
		friend class GameActionManager;
	protected:
		GameActionSound( const std::string& name,
			const XmlReader::XmlReaderIterator& actItr );
	public:
		virtual ~GameActionSound();
		virtual void OnCreate();
		virtual void OnActivate();
		virtual void OnDeactivate();
		virtual GameAction* Copy() const;

		enum SoundLoopMode
		{
			GA_PLAY_END,
			GA_PLAY_STOP,
			GA_LOOP_END,
			GA_LOOP_STOP,
			GA_PLAY_MUSIC,
			GA_STOP_MUSIC
		};

	protected:
		SoundLoopMode mMode;
		std::string mSoundName;
		SoundClip* mClip;
		int mLoops;
	};
	//---------------------------------------
}