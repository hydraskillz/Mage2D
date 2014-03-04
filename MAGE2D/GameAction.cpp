#include "MageLib.h"

using namespace mage;


//---------------------------------------
static const char* GameActionTypeToCString( int type )
{
	static const char* GameActionTypeToCStringMapping[ GameAction::GA_TYPE_COUNT ] =
	{
		"Property",
		"AddObject",
		"Motion",
		"LoadLevel",
		"SendEvent",
		"State",
		"RemoveObject",
		"RestartLevel",
		"Camera",
		"Animation",
		"Physics",
	};
	if ( type >= GameAction::GA_PROPERTY && type < GameAction::GA_TYPE_COUNT )
	{
		return GameActionTypeToCStringMapping[ type ];
	}
	return "UNKNOWN";
}
//---------------------------------------


//---------------------------------------
// GameActionManager
//---------------------------------------
std::map< std::string, GameAction* > GameActionManager::mGameActions;
//---------------------------------------
void GameActionManager::CreateAction( const std::string& ownerName,
	const XmlReader::XmlReaderIterator& actItr )
{
	static bool _init_tables = false;
	static std::map< std::string, int > ActionTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		ActionTypeToEnum[ "Property"     ] = GameAction::GA_PROPERTY;
		ActionTypeToEnum[ "AddObject"    ] = GameAction::GA_ADDOBJECT;
		ActionTypeToEnum[ "Motion"       ] = GameAction::GA_MOTION;
		ActionTypeToEnum[ "LoadLevel"    ] = GameAction::GA_LOADLEVEL;
		ActionTypeToEnum[ "SendEvent"    ] = GameAction::GA_SEND_EVENT;
		ActionTypeToEnum[ "State"        ] = GameAction::GA_STATE;
		ActionTypeToEnum[ "RemoveObject" ] = GameAction::GA_REMOVEOBJECT;
		ActionTypeToEnum[ "RestartLevel" ] = GameAction::GA_RESTART_LEVEL;
		ActionTypeToEnum[ "Camera"       ] = GameAction::GA_CAMERA;
		ActionTypeToEnum[ "Animation"    ] = GameAction::GA_ANIMATION;
		ActionTypeToEnum[ "Physics"      ] = GameAction::GA_PHYSICS;
		ActionTypeToEnum[ "Sound"        ] = GameAction::GA_SOUND;
	}

	std::string name, type;

	// Name is stored as Owner::TriggerName for scoping reasons
	name = ownerName + "::" + actItr.GetAttributeAsString( "name" );
	type = actItr.GetAttributeAsString( "type" );

	GameAction*& gameAction = mGameActions[ name ];

	if ( gameAction )
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: '%s' already defined. Duplicate not loaded. Please make sure your Action names are unique per GameObject.\n"
			, name.c_str() );
		return;
	}

	switch ( GetMappedValue( ActionTypeToEnum, type, "Unknown Action type. No Action created.", -1 ) )
	{
		case GameAction::GA_PROPERTY:
		{
			gameAction = new GameActionProperty( name, actItr );
			break;
		}
		case GameAction::GA_ADDOBJECT:
		{
			gameAction = new GameActionAddObject( name, actItr );
			break;
		}
		case GameAction::GA_MOTION:
		{
			gameAction = new GameActionMotion( name, actItr );
			break;
		}
		case GameAction::GA_LOADLEVEL:
		{
			gameAction = new GameActionLoadLevel( name, actItr );
			break;
		}
		case GameAction::GA_SEND_EVENT:
		{
			gameAction = new GameActionSendEvent( name, actItr );
			break;
		}
		case GameAction::GA_STATE:
		{
			gameAction = new GameActionState( name, actItr );
			break;
		}
		case GameAction::GA_REMOVEOBJECT:
		{
			gameAction = new GameActionRemoveObject( name, actItr );
			break;
		}
		case GameAction::GA_RESTART_LEVEL:
		{
			gameAction = new GameActionRestartLevel( name, actItr );
			break;
		}
		case GameAction::GA_CAMERA:
		{
			gameAction = new GameActionCamera( name, actItr );
			break;
		}
		case GameAction::GA_ANIMATION:
		{
			gameAction = new GameActionAnimation( name, actItr );
			break;
		}
		case GameAction::GA_PHYSICS:
		{
			gameAction = new GameActionPhysics( name, actItr );
			break;
		}
		case  GameAction::GA_SOUND:
		{
			gameAction = new GameActionSound( name, actItr );
			break;
		}
		// Cannot create an action we don't know about...
		default:
			return;
	}
}
//---------------------------------------
GameAction* GameActionManager::NewAction( const std::string& name, GameObject* owner )
{
	GameAction* act = mGameActions[ name ];
	if ( act )
	{
		GameAction* actCopy = act->Copy();
		actCopy->AttachTo( owner, act->GetExecPriority() );
		actCopy->OnCreate();
		return actCopy;
	}
	return act;
}
//---------------------------------------
void GameActionManager::DestroyAction( const GameTrigger& trigger )
{
	assertion( false, "Not implemented" );
}
//---------------------------------------
void GameActionManager::DestroyAllActions()
{
	DestroyMapByValue( mGameActions );
}
//---------------------------------------


//---------------------------------------
// GameAction
//---------------------------------------
GameAction::GameAction( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameLogicNode( name )
{
#ifdef DEBUG_GAMEOBJECT_VERBOSE
	ConsolePrintf( C_FG_LIGHT_BLUE,
		"Loaded GameAction: %s\n"
		, mName.c_str() );
#endif
}
//---------------------------------------
GameAction::~GameAction()
{}
//---------------------------------------


//---------------------------------------
// GameActionProperty
//---------------------------------------
GameActionProperty::GameActionProperty( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "Add"     ] = GameActionProperty::PROPMANIP_ADD;
		PropertyTypeToEnum[ "Assign"  ] = GameActionProperty::PROPMANIP_ASSIGN;
	}

	std::string propManipType;

	mPropertyName   = actItr.GetAttributeAsString( "propertyName" );
	mValue          = actItr.GetAttributeAsString( "value" );
	propManipType   = actItr.GetAttributeAsString( "propertyManipulationMode" );
	mPropManipType  = (PropertyManipulationType) GetMappedValue( PropertyTypeToEnum, propManipType, "Invalid property manipulation mode", 0 );

	mType = GA_PROPERTY;

#ifdef DEBUG_GAMEOBJECT_VERBOSE
	static const char* manipModeStringMapping[] =
	{
		"NULL",
		"Add",
		"Assign"
	};

	ConsolePrintf( C_FG_LIGHT_BLUE,
		" Type: %s\n"		\
		" PropName: %s\n"	\
		" Value: %s\n"		\
		" ManipMode: %s\n"
		, GameActionTypeToCString( mType )
		, mPropertyName.c_str()
		, mValue.c_str()
		, manipModeStringMapping[ mPropManipType ] );
#endif
}
//---------------------------------------
GameActionProperty::~GameActionProperty()
{}
//---------------------------------------
void GameActionProperty::OnActivate()
{
	//ConsolePrintf( CONSOLE_INFO, "%s: %s Activated!\n"
	//	, mOwner->Name.c_str()
	//	, mName.c_str() );

	GameVar& prop = mOwner->GetProperty( mPropertyName );
	std::string value = EvaluateVariablePropertyString( mValue );

	if ( prop == GameVar::NULL_VAR )
	{
		ConsolePrintf( CONSOLE_ERROR, "%s : Property '%s' not found! Please make sure the property referenced exists.\n"
			, mName.c_str()
			, mPropertyName.c_str() );
		return;
	}

	if ( mPropManipType == PROPMANIP_ADD )
	{
		prop += GameVar( NULL, prop.GetType(), value );
	}
	else if ( mPropManipType == PROPMANIP_ASSIGN )
	{
		prop = GameVar( NULL, prop.GetType(), value );
	}
	
	//ConsolePrintf( CONSOLE_INFO, "%s\n"
	//	, prop.ToString().c_str() );
}
//---------------------------------------
GameAction* GameActionProperty::Copy() const
{
	return new GameActionProperty( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionAddObject
//---------------------------------------
GameActionAddObject::GameActionAddObject( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_ADDOBJECT;
	mObjectToAdd    = actItr.GetAttributeAsString( "objectName" );
	mAttachToParent = actItr.GetAttributeAsBool( "attach", false );
	mSpawnOffset    = actItr.GetAttributeAsVec2f( "spawnOffset", Vec2f::ZERO );
	mSpawnVelocity  = actItr.GetAttributeAsVec2f( "spawnVelocity", Vec2f::ZERO );
}
//---------------------------------------
GameActionAddObject::~GameActionAddObject()
{}
//---------------------------------------
void GameActionAddObject::OnActivate()
{
	Dictionary params;
	std::string objectName;
	objectName = EvaluateVariablePropertyString( mObjectToAdd );
	params.Set( "ObjectName", objectName );
	params.Set( "Position", ( mAttachToParent ? Vec2f::ZERO : mOwner->Position ) + mSpawnOffset );
	EventManager::FireEvent( "SpawnGameObject", params );

	if ( mAttachToParent )
	{
		GameObject* spawned;

		params.Get( "GameObject", spawned );

		if ( spawned )
		{
			spawned->SetParent( mOwner );
			spawned->SetLinearVelocity( mSpawnVelocity );
		}
	}
}
//---------------------------------------
GameAction* GameActionAddObject::Copy() const
{
	return new GameActionAddObject( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionMotion
//---------------------------------------
GameActionMotion::GameActionMotion( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_MOTION;

	mMotion[ MOTION_MOVE     ] = actItr.GetAttributeAsVec2f( "move", Vec2f::ZERO );
	mMotion[ MOTION_FORCE    ] = actItr.GetAttributeAsVec2f( "force", Vec2f::ZERO );
	mMotion[ MOTION_VELOCITY ] = actItr.GetAttributeAsVec2f( "velocity", Vec2f::ZERO );
}
//---------------------------------------
GameActionMotion::~GameActionMotion()
{}
//---------------------------------------
void GameActionMotion::OnActivate()
{
	// Static motion
	mOwner->ApplyMovement( mMotion[ MOTION_MOVE ] );
	// External force motion
	mOwner->ApplyForce( mMotion[ MOTION_FORCE ] );
	// Set linear velocity
	if ( mMotion[ MOTION_VELOCITY ] != Vec2f::ZERO )
	{
		//mOwner->SetLinearVelocity( mMotion[ MOTION_VELOCITY ] );
		mOwner->ApplyLinearVelocity( mMotion[ MOTION_VELOCITY ] );
	}
}
//---------------------------------------
GameAction* GameActionMotion::Copy() const
{
	return new GameActionMotion( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionLoadLevel
//---------------------------------------
GameActionLoadLevel::GameActionLoadLevel( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_LOADLEVEL;
	mLevelName = actItr.GetAttributeAsString( "levelName" );
}
//---------------------------------------
GameActionLoadLevel::~GameActionLoadLevel()
{}
//---------------------------------------
void GameActionLoadLevel::OnActivate()
{
	GameInfo& gameInfo = GameInfo::GetGameInfo();

	gameInfo.CurrentLevel = EvaluateVariablePropertyString( mLevelName );
	gameInfo.ShouldLoadLevel = true;
}
//---------------------------------------
GameAction* GameActionLoadLevel::Copy() const
{
	return new GameActionLoadLevel( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionSendEvent
//---------------------------------------
GameActionSendEvent::GameActionSendEvent( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_SEND_EVENT;

	mEventName = actItr.GetAttributeAsString( "eventName" );
}
//---------------------------------------
GameActionSendEvent::~GameActionSendEvent()
{}
//---------------------------------------
void GameActionSendEvent::OnActivate()
{
	std::string eventName = EvaluateVariablePropertyString( mEventName );
	if ( !eventName.empty() )
	{
		ConsolePrintf( C_FG_LIGHT_PURPLE, "Event : %s\n", EvaluateVariablePropertyString( mEventName ).c_str() );
		EventManager::FireEvent( EvaluateVariablePropertyString( mEventName ) );
	}
}
//---------------------------------------
GameAction* GameActionSendEvent::Copy() const
{
	return new GameActionSendEvent( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionState
//---------------------------------------
GameActionState::GameActionState( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
	, mStateMask( 0U )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "Set"      ] = STATE_OP_SET;
		PropertyTypeToEnum[ "Add"      ] = STATE_OP_ADD;
		PropertyTypeToEnum[ "Subtract" ] = STATE_OP_SUB;
		PropertyTypeToEnum[ "Invert"   ] = STATE_OP_INV;
	}

	mOpMode = (StateOperationMode) PropertyTypeToEnum[ actItr.GetAttributeAsString( "operation" ) ];
	actItr.GetAttributeAsCSV( "state", mStateNames );
}
//---------------------------------------
GameActionState::~GameActionState()
{}
//---------------------------------------
void GameActionState::OnCreate()
{
	std::vector< int > bits;
	GameObjectDefinition* def = GameObjectDefinition::GetDefinitionByName( mOwner->Name );
	for ( auto itr = mStateNames.begin(); itr != mStateNames.end(); ++itr )
	{
		bits.push_back( def->GetStateIdFromName( *itr ) );
	}
	for ( auto itr = bits.begin(); itr != bits.end(); ++itr )
	{
		int b = *itr;
		if ( b < 0 || b > 30 )
		{
			ConsolePrintf( CONSOLE_ERROR, "Invalid state '%d'. Value must be in [1,30]\n", b );
		}
		SetBit( mStateMask, (uint8) b );
	}
	ConsolePrintf( CONSOLE_INFO, "StateMask = %u [ %s ]\n", mStateMask, def->GetStateNameFromId( mStateMask ).c_str() );
}
//---------------------------------------
void GameActionState::OnActivate()
{
	switch ( mOpMode )
	{
		case STATE_OP_SET:
		{
			mOwner->StateMask = mStateMask;
			break;
		}
		case STATE_OP_ADD:
		{
			mOwner->StateMask |= mStateMask;
			break;
		}
		case STATE_OP_SUB:
		{
			mOwner->StateMask &= ~mStateMask;
			break;
		}
		case STATE_OP_INV:
		{
			mOwner->StateMask ^= mStateMask;
			break;
		}
	}
	ConsolePrintf( C_FG_AQUA, "%s : State Set to %u\n"
		, mOwner->Name.c_str()
		, mOwner->StateMask
		);
}
//---------------------------------------
GameAction* GameActionState::Copy() const
{
	return new GameActionState( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionRemoveObject
//---------------------------------------
GameActionRemoveObject::GameActionRemoveObject( const std::string& name,
		const XmlReader::XmlReaderIterator& actItr )
		: GameAction( name, actItr )
{
	mType = GA_REMOVEOBJECT;
}
//---------------------------------------
GameActionRemoveObject::~GameActionRemoveObject()
{}
//---------------------------------------
void GameActionRemoveObject::OnActivate()
{
	mOwner->Invalidate();
}
//---------------------------------------
GameAction* GameActionRemoveObject::Copy() const
{
	return new GameActionRemoveObject( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionRestartLevel
//---------------------------------------
GameActionRestartLevel::GameActionRestartLevel( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_RESTART_LEVEL;
}
//---------------------------------------
GameActionRestartLevel::~GameActionRestartLevel()
{}
//---------------------------------------
void GameActionRestartLevel::OnActivate()
{
	GameInfo::GetGameInfo().ShouldLoadLevel = true;
}
//---------------------------------------
GameAction* GameActionRestartLevel::Copy() const
{
	return new GameActionRestartLevel( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionCamera
//---------------------------------------
GameActionCamera::GameActionCamera( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_CAMERA;

	mObjectToLookAt = actItr.GetAttributeAsString( "objectToLookAt" );
	Vec2i cameraLimitX = actItr.GetAttributeAsVec2f( "rangeX", Vec2f( 0, -1 ) );
	Vec2i cameraLimitY = actItr.GetAttributeAsVec2f( "rangeY", Vec2f( 0, -1 ) );
	mCameraBounds.Set( cameraLimitX.x, cameraLimitY.x,
		cameraLimitX.y, cameraLimitY.y );
}
//---------------------------------------
GameActionCamera::~GameActionCamera()
{}
//---------------------------------------
void GameActionCamera::OnActivate()
{
	GameObject* target = mOwner->GameMap->GetFirstObjectOfName( mObjectToLookAt );
	if ( target )
	{
		mOwner->GameMap->SetCameraTarget( target );
	}

	// Use full width
	if ( mCameraBounds.Right < 0 )
	{
		mCameraBounds.Right = mOwner->GameMap->GetWorldBounds().Right;
	}
	// Use full height
	if ( mCameraBounds.Bottom < 0 )
	{
		mCameraBounds.Bottom = mOwner->GameMap->GetWorldBounds().Bottom;
	}

	mOwner->GameMap->SetCameraBounds( mCameraBounds );
}
//---------------------------------------
GameAction* GameActionCamera::Copy() const
{
	return new GameActionCamera( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionAnimation
//---------------------------------------
GameActionAnimation::GameActionAnimation( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	mType = GA_ANIMATION;

	mAnimName = actItr.GetAttributeAsString( "animName" );
}
//---------------------------------------
GameActionAnimation::~GameActionAnimation()
{}
//---------------------------------------
void GameActionAnimation::OnActivate()
{
	Sprite* sprite = mOwner->GetSpriteByIndex( 0 );

	if ( sprite )
	{
		sprite->SetAnimationState( EvaluateVariablePropertyString( mAnimName ) );
		//mOwner->SetBoundsFromSprite();
	}
}
//---------------------------------------
GameAction* GameActionAnimation::Copy() const
{
	return new GameActionAnimation( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionPhysics
//---------------------------------------
GameActionPhysics::GameActionPhysics( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "SuspendDynamics" ] = GA_SUSPEND_DYNAMICS;
		PropertyTypeToEnum[ "RestoreDynamics" ] = GA_RESTORE_DYNAMICS;
		PropertyTypeToEnum[ "SetMass"         ] = GA_SET_MASS;
		PropertyTypeToEnum[ "SetGravity"      ] = GA_SET_GRAVITY;
	}

	mType = GA_PHYSICS;

	mMode = (PhysicsMode) PropertyTypeToEnum[ actItr.GetAttributeAsString( "mode" ) ];
	
	if ( mMode == GA_SET_MASS )
	{
		mMass = actItr.GetAttributeAsUInt( "mass" );
	}
	else if ( mMode == GA_SET_GRAVITY )
	{
		mGravity = actItr.GetAttributeAsVec2f( "gravity" );
	}
}
//---------------------------------------
GameActionPhysics::~GameActionPhysics()
{}
//---------------------------------------
void GameActionPhysics::OnActivate()
{
	switch ( mMode )
	{
		case GA_SUSPEND_DYNAMICS:
		{
			mOwner->SetPhysics( GameObject::PHYS_STATIC );
			break;
		}

		case GA_RESTORE_DYNAMICS:
		{
			mOwner->SetPhysics( GameObject::PHYS_DYNAMIC );
			break;
		}

		case GA_SET_MASS:
		{
			mOwner->SetMass( mMass );
			break;
		}

		case GA_SET_GRAVITY:
		{
			mOwner->GameMap->Gravity = mGravity;
		}
	}
}
//---------------------------------------
GameAction* GameActionPhysics::Copy() const
{
	return new GameActionPhysics( *this );
}
//---------------------------------------


//---------------------------------------
// GameActionSound
//---------------------------------------
GameActionSound::GameActionSound( const std::string& name,
	const XmlReader::XmlReaderIterator& actItr )
	: GameAction( name, actItr )
	, mClip( NULL )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "PlayEnd"   ] = GA_PLAY_END;
		PropertyTypeToEnum[ "PlayStop"  ] = GA_PLAY_STOP;
		PropertyTypeToEnum[ "LoopEnd"   ] = GA_LOOP_END;
		PropertyTypeToEnum[ "LoopStop"  ] = GA_LOOP_STOP;
		PropertyTypeToEnum[ "PlayMusic" ] = GA_PLAY_MUSIC;
		PropertyTypeToEnum[ "StopMusic" ] = GA_STOP_MUSIC;
	}

	mMode = (SoundLoopMode) PropertyTypeToEnum[ actItr.GetAttributeAsString( "mode" ) ];
	mSoundName = actItr.GetAttributeAsString( "soundName" );
	mLoops = actItr.GetAttributeAsInt( "loops", 0 );
}
//---------------------------------------
GameActionSound::~GameActionSound()
{}
//---------------------------------------
void GameActionSound::OnCreate()
{
	if ( mMode == GA_PLAY_MUSIC ||  mMode == GA_STOP_MUSIC )
	{
		mClip = SoundManager::LoadMusic( EvaluateVariablePropertyString( mSoundName ) );
	}
	else
	{
		mClip = SoundManager::LoadSFX( EvaluateVariablePropertyString( mSoundName ) );
	}
}
//---------------------------------------
void GameActionSound::OnActivate()
{
	if ( mClip )
	{
		if ( mMode == GA_STOP_MUSIC )
		{
			mClip->Stop();
		}
		else
		{
			mClip->Play( -1, mLoops );
		}
	}
}
//---------------------------------------
void GameActionSound::OnDeactivate()
{
}
//---------------------------------------
GameAction* GameActionSound::Copy() const
{
	return new GameActionSound( *this );
}
//---------------------------------------