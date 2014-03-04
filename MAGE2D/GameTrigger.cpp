#include "MageLib.h"

using namespace mage;

//---------------------------------------
// Debugging utility functions
static const char* GameTriggerTypeToCString( int type )
{
	static const char* GameTriggerTypeToCStringMapping[ GameTrigger::GT_TYPE_COUNT ] =
	{
		"Always",
		"Event",
		"Collision",
		"Property",
		"Animation"
	};

	if ( type >= GameTrigger::GT_ALWAYS && type < GameTrigger::GT_TYPE_COUNT )
	{
		return GameTriggerTypeToCStringMapping[ type ];
	}
}
//---------------------------------------


//---------------------------------------
// GameObjectTrigger
//---------------------------------------
std::map< std::string, GameTrigger* > GameTriggerManager::mGameTriggers;
//---------------------------------------
void GameTriggerManager::CreateTrigger( const std::string& ownerName, const XmlReader::XmlReaderIterator& trigItr )
{
	static bool _init_tables = false;
	static std::map< std::string, int > TriggerTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		TriggerTypeToEnum[ "Always"      ] = GameTrigger::GT_ALWAYS;
		TriggerTypeToEnum[ "Event"       ] = GameTrigger::GT_EVENT;
		TriggerTypeToEnum[ "Collision"   ] = GameTrigger::GT_COLLISION;
		TriggerTypeToEnum[ "Property"    ] = GameTrigger::GT_PROPERTY;
		TriggerTypeToEnum[ "Animation"   ] = GameTrigger::GT_ANIMATION;
	}

	std::string name, type;

	// Name is stored as Owner::TriggerName for scoping reasons
	name = ownerName + "::" + trigItr.GetAttributeAsString( "name" );
	type = trigItr.GetAttributeAsString( "type" );

	GameTrigger*& gameTrigger = mGameTriggers[ name ];

	if ( gameTrigger )
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: '%s' already defined. Duplicate not loaded. Please make sure your Trigger names are unique per GameObject.\n"
			, name.c_str() );
		return;
	}

	switch ( GetMappedValue( TriggerTypeToEnum, type, "Unknown Trigger type. No trigger created\n", -1 ) )
	{
		case GameTrigger::GT_ALWAYS:
		{
			gameTrigger = new GameTriggerAlways( name, trigItr );
			break;
		}
		case GameTrigger::GT_EVENT:
		{
			gameTrigger = new GameTriggerEvent( name, trigItr );
			break;
		}
		case GameTrigger::GT_COLLISION:
		{
			gameTrigger = new GameTriggerCollision( name, trigItr );
			break;
		}
		case GameTrigger::GT_PROPERTY:
		{
			gameTrigger = new GameTriggerProperty( name, trigItr );
			break;
		}
		case GameTrigger::GT_ANIMATION:
		{
			gameTrigger = new GameTriggerAnimation( name, trigItr );
			break;
		}
		// Cannot create a trigger we don't know about...
		default:
			return;
	}
}
//---------------------------------------
GameTrigger* GameTriggerManager::NewTrigger( const std::string& name, GameObject* owner )
{
	GameTrigger* trig = mGameTriggers[ name ];
	if ( trig )
	{
		GameTrigger* trigCopy = trig->Copy();
		trigCopy->AttachTo( owner, 0 );
		//trigCopy->OnCreate();
		return trigCopy;
	}
	return trig;
}
//---------------------------------------
void GameTriggerManager::DestroyAllTriggers()
{
	DestroyMapByValue( mGameTriggers );
}
//---------------------------------------


//---------------------------------------
// GameTrigger
//---------------------------------------
GameTrigger::GameTrigger( const std::string& name,
	const XmlReader::XmlReaderIterator& trigItr )
	: GameLogicNode( name )
	, mRepeatOk( true )
	, mLastTickFrame( 0 )
	, mIsActive( GT_INACTIVE )
{
	mTickDelay           = trigItr.GetAttributeAsInt( "logicTickDelay", 0 );
	mRepeatTrueSignal    = trigItr.GetAttributeAsBool( "repeatTrue", false );
	mRepeatFalseSignal   = trigItr.GetAttributeAsBool( "repeatFalse", false );
	mOnce                = trigItr.GetAttributeAsBool( "once", false );
	mInvertSignal        = trigItr.GetAttributeAsBool( "invert", false );

#ifdef DEBUG_GAMEOBJECT_VERBOSE
	ConsolePrintf( C_FG_LIGHT_AQUA,
		"Loaded GameTrigger: %s\n" \
		" Delay: %u\n"		\
		" Repeat: %s\n"	    \
		" Once: %s\n"		\
		" Invert: %s\n"		\
		, mName.c_str()
		, mTickDelay
		, BoolToCString( mRepeatTrueSignal )
		, BoolToCString( mOnce )
		, BoolToCString( mInvertSignal ) );
#endif
}
//---------------------------------------
//GameTrigger::GameTrigger( const GameTrigger& other )
//	: GameLogicNode( other.mName )
//{
//	mType = other.mType;
//	mIsActive = other.mIsActive;
//	mTickDelay = other.mTickDelay;
//	mLastTickFrame = other.mLastTickFrame;
//	mRepeatTrueSignal = other.mRepeatTrueSignal;
//	mRepeatFalseSignal = other.mRepeatFalseSignal;
//	mOnce = other.mOnce;
//	mInvertSignal = other.mInvertSignal;
//	mRepeatOk = other.mRepeatOk;
//}
//---------------------------------------
GameTrigger::~GameTrigger()
{}
//---------------------------------------
void GameTrigger::OnUpdate( float dt )
{
	mRepeatOk = true;
	if ( mRepeatTrueSignal && mTickDelay != 0 )
	{
		// First time is always true, once active repeat delay is applied
		if ( mIsActive == GT_INACTIVE || mIsActive == GT_JUST_DEACTIVATED )
		{
			mLastTickFrame = FrameRateControl::GetInstance().GetTotalFrames() + mTickDelay;
		}

		mRepeatOk = FrameRateControl::GetInstance().GetTotalFrames() - mLastTickFrame >= mTickDelay;
		if ( mRepeatOk )
		{
			mLastTickFrame = FrameRateControl::GetInstance().GetTotalFrames();
		}
	}
}
//---------------------------------------
void GameTrigger::Reset()
{
	mIsActive = GameTrigger::GT_INACTIVE;
	mLastTickFrame = FrameRateControl::GetInstance().GetTotalFrames();
}
//---------------------------------------


//---------------------------------------
// GameTriggerAlways
//---------------------------------------
GameTriggerAlways::GameTriggerAlways( const std::string& ownerName,
	const XmlReader::XmlReaderIterator& trigItr )
	: GameTrigger( ownerName, trigItr )
{
	mType = GameTrigger::GT_ALWAYS;

#ifdef DEBUG_GAMEOBJECT_VERBOSE
	ConsolePrintf( C_FG_LIGHT_AQUA,
		" Type: %s\n"
		, GameTriggerTypeToCString( mType ) );
#endif
}
//---------------------------------------
//GameTriggerAlways::GameTriggerAlways( const GameTriggerAlways& other )
//	: GameTrigger( *this )
//{}
//---------------------------------------
GameTriggerAlways::~GameTriggerAlways()
{}
//---------------------------------------
void GameTriggerAlways::OnUpdate( float dt )
{
	GameTrigger::OnUpdate( dt );

	// TriggerAlways is always true
	if ( mIsActive == GT_INACTIVE )
	{
		mIsActive = GT_JUST_ACTIVATED;
	}

	else if ( mIsActive == GT_JUST_ACTIVATED )
	{
		mIsActive = GT_ACTIVATED;
	}
}
//---------------------------------------
GameTrigger* GameTriggerAlways::Copy() const
{
	return new GameTriggerAlways( *this );
}
//---------------------------------------


//---------------------------------------
// GameTriggerEvent
//---------------------------------------
GameTriggerEvent::GameTriggerEvent( const std::string& name,
		const XmlReader::XmlReaderIterator& trigItr )
	: GameTrigger( name, trigItr )
	, EventName( trigItr.GetAttributeAsString( "eventName" ) )
	, mRecieved( false )
{
	mType = GameTrigger::GT_EVENT;
}
//---------------------------------------
//GameTriggerEvent::GameTriggerEvent( const GameTriggerEvent& other )
//	: GameTrigger( *this )
//	, EventName( other.EventName )
//{}
//---------------------------------------
GameTriggerEvent::~GameTriggerEvent()
{
	EventManager::UnregisterObjectForAllEvent( *this );
}
//---------------------------------------
void GameTriggerEvent::OnUpdate( float dt )
{
	GameTrigger::OnUpdate( dt );

	if ( mIsActive == GameTrigger::GT_INACTIVE || mIsActive == GameTrigger::GT_ACTIVATED )
	{
		if ( mRecieved )
		{
			mIsActive = GameTrigger::GT_JUST_ACTIVATED;
			mRecieved = false;
		}
	}

	else if ( mIsActive == GameTrigger::GT_JUST_ACTIVATED )
	{
		mIsActive = GameTrigger::GT_ACTIVATED;
	}
	// This is potentially very slow... but would allow the listened message to be changed dynamically
	/*
	std::string eventName = EvaluateVariablePropertyString( EventName );
	if ( eventName != mRegisteredEvent )
	{
		EventManager::UnregisterObjectForEvent( mRegisteredEvent, *this );
		mRegisteredEvent = EvaluateVariablePropertyString( EventName );
		EventManager::RegisterObjectForEvent( mRegisteredEvent, *this, &GameTriggerEvent::EventReceived );
	}*/
}
//---------------------------------------
void GameTriggerEvent::OnCreate()
{
	mRegisteredEvent = EvaluateVariablePropertyString( EventName );
	EventManager::RegisterObjectForEvent( mRegisteredEvent, *this, &GameTriggerEvent::EventReceived );
}
//---------------------------------------
GameTrigger* GameTriggerEvent::Copy() const
{
	return new GameTriggerEvent( *this );
}
//---------------------------------------
void GameTriggerEvent::Reset()
{
	GameTrigger::Reset();

	EventManager::UnregisterObjectForAllEvent( *this );
	OnCreate();
}
//---------------------------------------
void GameTriggerEvent::EventReceived( Dictionary& params )
{
	mRecieved = true;
}
//---------------------------------------


//---------------------------------------
// GameTriggerCollision
//---------------------------------------
GameTriggerCollision::GameTriggerCollision( const std::string& name,
	const XmlReader::XmlReaderIterator& trigItr )
	: GameTrigger( name, trigItr )
{
	mType = GT_COLLISION;
	mProperty = trigItr.GetAttributeAsString( "objectProperty", "" );
}
//---------------------------------------
GameTriggerCollision::~GameTriggerCollision()
{}
//---------------------------------------
void GameTriggerCollision::OnUpdate( float dt )
{
	GameTrigger::OnUpdate( dt );

	std::string prop = EvaluateVariablePropertyString( mProperty );
	const std::set< GameObject* >& collidingObjects = mOwner->GetCollidingObjects();

	// Check if the set of colliding objects matches the trigger specs
	for ( auto itr = collidingObjects.begin(); itr != collidingObjects.end(); ++itr )
	{
		GameObject& gameObject = **itr;

		// No property to look for (all) or property was specified
		if ( prop.length() == 0 || gameObject.HasProperty( prop ) )
		{
			if ( mIsActive == GT_JUST_ACTIVATED )
			{
				mIsActive = GT_ACTIVATED;
			}
			else if ( mIsActive != GT_ACTIVATED )
			{
				mIsActive = GT_JUST_ACTIVATED;
			}
			// Don't need to check the other objects
			return;
		}
	}

	if ( mIsActive == GT_JUST_ACTIVATED || mIsActive == GT_ACTIVATED )
	{
		mIsActive = GT_JUST_DEACTIVATED;
	}
	else if ( mIsActive == GT_JUST_DEACTIVATED )
	{
		mIsActive = GT_INACTIVE;
	}
}
//---------------------------------------
GameTrigger* GameTriggerCollision::Copy() const
{
	return new GameTriggerCollision( *this );
}
//---------------------------------------


//---------------------------------------
// GameTriggerProperty
//---------------------------------------
GameTriggerProperty::GameTriggerProperty( const std::string& name,
	const XmlReader::XmlReaderIterator& trigItr )
	: GameTrigger( name, trigItr )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "Equal"     ] = GT_PROP_EQUAL;
		PropertyTypeToEnum[ "NotEqual"  ] = GT_PROP_NOT_EQUAL;
		PropertyTypeToEnum[ "Changed"   ] = GT_PROP_CHANGED;
		PropertyTypeToEnum[ "Interval"  ] = GT_PROP_INTERVAL;
	}

	mType = GT_PROPERTY;
	mProperty    = trigItr.GetAttributeAsString( "property" );
	mCompareType = (PropertyComparisonType) PropertyTypeToEnum[ trigItr.GetAttributeAsString( "compareMode" ) ];

	if ( mCompareType == GT_PROP_EQUAL || mCompareType == GT_PROP_NOT_EQUAL )
	{
		mValue = trigItr.GetAttributeAsString( "value" );
	}
	else if ( mCompareType == GT_PROP_INTERVAL )
	{
		mValue  = trigItr.GetAttributeAsString( "min" );
		mValue2 = trigItr.GetAttributeAsString( "max" );
	}

	mBadProperty = false;
}
//---------------------------------------
GameTriggerProperty::~GameTriggerProperty()
{}
//---------------------------------------
void GameTriggerProperty::OnUpdate( float dt )
{
	GameTrigger::OnUpdate( dt );

	if ( mBadProperty ) return;

	std::string prop = EvaluateVariablePropertyString( mProperty );
	std::string val  = EvaluateVariablePropertyString( mValue );
	bool test = false;
	GameVar& gvar = mOwner->GetProperty( prop );

	if ( gvar == GameVar::NULL_VAR )
	{
		ConsolePrintf( CONSOLE_ERROR, "%s : Bad or missing property: '%s'\n", mName.c_str(), prop.c_str() );
		mBadProperty = true;
		return;
	}

	// Check property
	switch ( mCompareType )
	{
		case GT_PROP_EQUAL:
		{
			test = gvar == GameVar( NULL, gvar.GetType(), val );
			break;
		}
		case GT_PROP_NOT_EQUAL:
		{
			test = gvar != GameVar( NULL, gvar.GetType(), val );
			break;
		}
		case GT_PROP_CHANGED:
		{
			test = gvar != GameVar( NULL, gvar.GetType(), mValue );
			if ( test )
			{
				mValue = gvar.ValueString();
			}
			break;
		}
		case GT_PROP_INTERVAL:
		{
			test = gvar >= GameVar( NULL, gvar.GetType(), mValue ) &&
				   gvar <= GameVar( NULL, gvar.GetType(), mValue2 );
			break;
		}
	}

	// Property check passed, activate
	if ( test )
	{
		if ( mIsActive == GT_INACTIVE )
		{
			mIsActive = GT_JUST_ACTIVATED;
		}
		else if ( mIsActive != GT_ACTIVATED )
		{
			mIsActive = GT_ACTIVATED;
		}
	}
	else
	{
		if ( mIsActive == GT_JUST_ACTIVATED || mIsActive == GT_ACTIVATED )
		{
			mIsActive = GT_JUST_DEACTIVATED;
		}
		else if ( mIsActive == GT_JUST_DEACTIVATED )
		{
			mIsActive = GT_INACTIVE;
		}
	}
}
//---------------------------------------
void GameTriggerProperty::OnCreate()
{
	if ( mCompareType == GT_PROP_CHANGED )
	{
		std::string prop = EvaluateVariablePropertyString( mProperty );
		GameVar& gvar = mOwner->GetProperty( prop );
		mValue = gvar.ValueString();
	}
}
//---------------------------------------
GameTrigger* GameTriggerProperty::Copy() const
{
	return new GameTriggerProperty( *this );
}
//---------------------------------------


//---------------------------------------
// GameTriggerAnimation
//---------------------------------------
GameTriggerAnimation::GameTriggerAnimation( const std::string& name,
	const XmlReader::XmlReaderIterator& trigItr )
	: GameTrigger( name, trigItr )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "Complete"     ] = GA_ANIM_COMPLETE;
	}

	mType = GT_ANIMATION;
	mAnimName    = trigItr.GetAttributeAsString( "animName" );
	mCompareType = (AnimCheckMode) PropertyTypeToEnum[ trigItr.GetAttributeAsString( "mode" ) ];

}
//---------------------------------------
GameTriggerAnimation::~GameTriggerAnimation()
{}
//---------------------------------------
void GameTriggerAnimation::OnUpdate( float dt )
{
	GameTrigger::OnUpdate( dt );

	if ( mCompareType == GA_ANIM_COMPLETE )
	{
		if ( mOwner->GetSpriteByIndex( 0 )->GetCurrentAnimationName() == EvaluateVariablePropertyString( mAnimName )
			&& mOwner->GetSpriteByIndex( 0 )->IsCurrentAnimationFinished() )
		{
			if ( mIsActive == GT_INACTIVE || GT_JUST_DEACTIVATED )
			{
				mIsActive = GT_JUST_ACTIVATED;
			}
			else if ( mIsActive == GT_JUST_ACTIVATED )
			{
				mIsActive = GT_ACTIVATED;
			}
			return;
		}
	}

	if ( mIsActive == GT_JUST_ACTIVATED || mIsActive == GT_ACTIVATED )
	{
		mIsActive = GT_JUST_DEACTIVATED;
	}
	else if ( mIsActive == GT_JUST_DEACTIVATED )
	{
		mIsActive = GT_INACTIVE;
	}
}
//---------------------------------------
GameTrigger* GameTriggerAnimation::Copy() const
{
	return new GameTriggerAnimation( *this );
}
//---------------------------------------