#include "MageLib.h"

using namespace mage;

DECLARE_DEFINITION_BASE_MEMBERS( GameObjectDefinition, GameObject );

//---------------------------------------
GameObjectDefinition::GameObjectDefinition( const std::string& filename )
{
	XmlReader reader( filename.c_str() );
	XmlReader::XmlReaderIterator rootItr = reader.ReadRoot();

	if ( rootItr.IsValid() )
	{
		// Don't create Definitions for abstract objects
		if ( !rootItr.GetAttributeAsBool( "abstract", false ) )
		{
			rootItr.ValidateXMLAttributes( "name", "initialState,physics,collision,debugColor,debugDraw,collisionBox,abstract" );
			rootItr.ValidateXMLChildElemnts( "", "Trigger,Controller,Action,Properties,Sprite,Text,Include,State" );

			mName          = rootItr.GetAttributeAsString( "name" );
			mPhysicsType   = GameObject::GetPhysicsTypeFromString(
								rootItr.GetAttributeAsString( "physics", "static" ) );
			mCollisionType = GameObject::GetCollisionTypeFromString(
								rootItr.GetAttributeAsString( "collision", "all" ) );
			mDebugColor    = rootItr.GetAttributeAsColor( "debugColor", Color::LIGHT_GREY );
			mDebugDraw     = rootItr.GetAttributeAsBool( "debugDraw", false );
			Vec2f bounds   = rootItr.GetAttributeAsVec2f( "collisionBox", Vec2f::ZERO );

			mBoundingRect.Set( 0, 0, bounds.x, bounds.y );

			mActiveStateId = 0;
			mStateNameMapping[ mActiveStateId ] = "__global__";

			std::vector< std::string > stateNames;
			rootItr.GetAttributeAsCSV( "initialState", stateNames, "__global__" );
			std::vector< int > bits;
			for ( auto itr = stateNames.begin(); itr != stateNames.end(); ++itr )
			{
				bits.push_back( GetStateIdFromName( *itr ) );
			}
			mInitialState = 0U;
			for ( auto itr = bits.begin(); itr != bits.end(); ++itr )
			{
				int b = *itr;
				if ( b < 0 || b > 30 )
				{
					ConsolePrintf( CONSOLE_ERROR, "Invalid state '%d'. Value must be in [1,30]\n", b );
				}
				SetBit( mInitialState, (uint8) b );
			}

			LoadObject( rootItr );
		}
	}

	RegisterDefinition( mName );
}
//---------------------------------------
GameObjectDefinition::~GameObjectDefinition()
{
	DestroyMapByValue( mProperties );
	DestroyVector( mControllerDefinitions );
	DestroyVector( mSpriteDefinitions );
	DestroyVector( mGameStrings );
}
//---------------------------------------
GameObject* GameObjectDefinition::Create() const
{
	GameObject* gameObject = new GameObject( 
		mName,
		mProperties,
		mInitialState,
		(GameObject::ObjectPhysicsType) mPhysicsType,
		(GameObject::ObjectCollisionFlags) mCollisionType );

	gameObject->DebugColor = mDebugColor;
	gameObject->DebugDraw = mDebugDraw;
	gameObject->BoundingRect = mBoundingRect;

#if 1
	ConsolePrintf( "Created GameObject '%s' [ state='%s' ]\n"
		, mName.c_str()
		, GetStateNameFromId( mInitialState ).c_str() );
#endif

	// Create Sprites
	for ( auto itr = mSpriteDefinitions.begin(); itr != mSpriteDefinitions.end(); ++itr )
	{
		SpriteDefinitionInfo& spriteInfo = **itr;
		Sprite* sprite = SpriteManager::Manager.CreateSprite( spriteInfo.mAnimName, spriteInfo.mPosition, spriteInfo.mInitialAnimName );
		sprite->RelativeToCamera = !spriteInfo.mSpriteSpace;
		sprite->Owner = gameObject;
		gameObject->mSprites.push_back( sprite );
	}

	for ( auto itr = mGameStrings.begin(); itr != mGameStrings.end(); ++itr )
	{
		gameObject->mGameStrings.push_back( new GameString( **itr ) );
	}

	// Create controllers
	for ( auto itr = mControllerDefinitions.begin(); itr != mControllerDefinitions.end(); ++itr )
	{
		const ControllerDefinition& contDef = **itr;
		GameController* cont = NULL;

		switch ( contDef.mType )
		{
			case GameController::GC_AND:
			{
				cont = new GameControllerAnd( contDef.mName, contDef.mState );
				break;
			}
			case GameController::GC_OR:
			{
				cont = new GameControllerOr( contDef.mName, contDef.mState );
				break;
			}
			case GameController::GC_NAND:
			{
				cont = new GameControllerNAnd( contDef.mName, contDef.mState );
				break;
			}
			case GameController::GC_NOR:
			{
				cont = new GameControllerNOr( contDef.mName, contDef.mState );
				break;
			}
			case GameController::GC_XOR:
			{
				cont = new GameControllerXOr( contDef.mName, contDef.mState );
				break;
			}
			case GameController::GC_XNOR:
			{
				cont = new GameControllerXNOr( contDef.mName, contDef.mState );
				break;
			}
			case GameController::GC_EXPR:
			{
				cont = new GameControllerExpr( contDef.mName, contDef.mState, contDef.mExtra );
				break;
			}
			case GameController::GC_SCRIPT:
			{
				cont = new GameControllerScript( contDef.mName, contDef.mState, contDef.mExtra );
				break;
			}
			default:
				break;
		}

		// Link triggers
		for ( auto trigItr = contDef.mTriggerNames.begin(); trigItr != contDef.mTriggerNames.end(); ++trigItr )
		{
			GameTrigger* trig = GameTriggerManager::NewTrigger( *trigItr, gameObject );
			if ( trig )
			{
				cont->AddTrigger( trig );
				gameObject->AddTrigger( trig );
			}
			else
			{
				ConsolePrintf( CONSOLE_WARNING, "Object '%s': Unable to link trigger '%s' to controller '%s'\n",
					mName.c_str(),
					trigItr->c_str(),
					contDef.mName.c_str() );
			}
		}

		// Link actions
		for ( auto actItr = contDef.mActionNames.begin(); actItr != contDef.mActionNames.end(); ++actItr )
		{
			GameAction* act = GameActionManager::NewAction( *actItr, gameObject );
			if ( act )
			{
				cont->AddAction( act );
				gameObject->AddAction( act );
			}
			else
			{
				ConsolePrintf( CONSOLE_WARNING, "Object '%s': Unable to link action '%s' to controller '%s'\n",
					mName.c_str(),
					actItr->c_str(),
					contDef.mName.c_str() );
			}
		}

		// Add controller
		if ( cont )
		{
			cont->AttachTo( gameObject, 0 );
			gameObject->AddController( cont );
		}

	}

	return gameObject;
}
//---------------------------------------
void GameObjectDefinition::LoadObject( const XmlReader::XmlReaderIterator& rootItr )
{
	// Include data
	for ( XmlReader::XmlReaderIterator inclItr = rootItr.NextChild( "Include" );
		inclItr.IsValid(); inclItr = inclItr.NextSibling( "Include" ) )
	{
		LoadInclude( inclItr );
	}

	// Sprite data
	for ( XmlReader::XmlReaderIterator sprItr = rootItr.NextChild( "Sprite" );
		sprItr.IsValid(); sprItr = sprItr.NextSibling( "Sprite" ) )
	{
		LoadSpriteData( sprItr );
	}

	// Text data
	for ( XmlReader::XmlReaderIterator textItr = rootItr.NextChild( "Text" );
		textItr.IsValid(); textItr = textItr.NextSibling( "Text" ) )
	{
		LoadTextData( textItr );
	}

	// Properties
	for ( XmlReader::XmlReaderIterator propsItr = rootItr.NextChild( "Properties" );
		propsItr.IsValid(); propsItr = propsItr.NextSibling( "Properties" ) )
	{
		LoadProperties( propsItr );
	}

	// Triggers
	for ( XmlReader::XmlReaderIterator trigItr = rootItr.NextChild( "Trigger" );
		trigItr.IsValid(); trigItr = trigItr.NextSibling( "Trigger" ) )
	{
		GameTriggerManager::CreateTrigger( mName, trigItr );
	}

	// Actions
	for ( XmlReader::XmlReaderIterator actItr = rootItr.NextChild( "Action" );
		actItr.IsValid(); actItr = actItr.NextSibling( "Action" ) )
	{
		GameActionManager::CreateAction( mName, actItr );
	}

	// Controllers
	for ( XmlReader::XmlReaderIterator conItr = rootItr.NextChild( "Controller" );
		conItr.IsValid(); conItr = conItr.NextSibling( "Controller" ) )
	{
		LoadControllerDefinition( conItr );
	}


	// States
	for ( XmlReader::XmlReaderIterator stateItr = rootItr.NextChild( "State" );
		stateItr.IsValid(); stateItr = stateItr.NextSibling( "State" ) )
	{
		stateItr.ValidateXMLAttributes( "name", "" );
		stateItr.ValidateXMLChildElemnts( "", "Trigger,Controller,Action,Sprite,Text" );

		mActiveStateId = GetStateIdFromName( stateItr.GetAttributeAsString( "name" ) );

		LoadObject( stateItr );
	}
}
//---------------------------------------
void GameObjectDefinition::LoadInclude( const XmlReader::XmlReaderIterator& inclItr )
{
	inclItr.ValidateXMLAttributes( "filename","" );

	std::string filename = inclItr.GetAttributeAsString( "filename" );

	XmlReader reader( filename.c_str() );
	XmlReader::XmlReaderIterator rootItr = reader.ReadRoot();

	if ( rootItr.IsValid() )
	{
		if ( rootItr.GetAttributeAsBool( "abstract", false ) )
		{
			LoadObject( rootItr );
		}
		else
		{
			ConsolePrintf( CONSOLE_WARNING, "%s : Cannot include non-abstract objects.\n"
				, filename.c_str() );
		}
	}
}
//---------------------------------------
void GameObjectDefinition::LoadSpriteData( const XmlReader::XmlReaderIterator& spriteItr )
{
	spriteItr.ValidateXMLAttributes( "animationSet","screenSpace,position,initialAnimName" );

	SpriteDefinitionInfo* spriteInfo = new SpriteDefinitionInfo();

	spriteInfo->mAnimName = spriteItr.GetAttributeAsString( "animationSet" );
	spriteInfo->mSpriteSpace = spriteItr.GetAttributeAsBool( "screenSpace", false );
	spriteInfo->mPosition = spriteItr.GetAttributeAsVec2f( "position", Vec2f::ZERO );
	spriteInfo->mInitialAnimName = spriteItr.GetAttributeAsString( "initialAnimName", "Idle" );

	// Force the sprite manager to load the animation
	std::string basePath = "sprites/";	// This may become a property maybe
	std::string animPath = basePath + spriteInfo->mAnimName;
	SpriteManager::Manager.LoadSpriteAnimations( animPath.c_str() );

	mSpriteDefinitions.push_back( spriteInfo );

	/*XmlReader::XmlReaderIterator cbItr = spriteItr.NextChild( "AnimationCallback" );
	if ( cbItr.IsValid() )
	{
		cbItr.ValidateXMLAttributes( "animationName", "onComplete" );

		std::string animName = cbItr.GetAttributeAsString( "animationName" );
		std::string onComplete = cbItr.GetAttributeAsString( "onComplete", "" );

		if ( onComplete.length() )
		{
			mAnimationEvents[ animName ] = onComplete;
		}
	}*/
}
//---------------------------------------
void GameObjectDefinition::LoadTextData( const XmlReader::XmlReaderIterator& textItr )
{
	textItr.ValidateXMLAttributes( "text", "time,position,color,screenSpace" );
	GameString* gString = new GameString();

	gString->Text = textItr.GetAttributeAsString( "text" );
	gString->Position = (Vec2i) textItr.GetAttributeAsVec2f( "position", Vec2f::ZERO );
	gString->TextColor = textItr.GetAttributeAsColor( "color", Color::WHITE );
	gString->TextFont = Font::DefaultFont;
	gString->WorldSpace = !textItr.GetAttributeAsBool( "screenSpace", true );

	mGameStrings.push_back( gString );
}
//---------------------------------------
void GameObjectDefinition::LoadProperties( const XmlReader::XmlReaderIterator& propsItr )
{
	// @TODO move this to a static function in GameVar
	static bool _init_tables = false;
	static std::map< std::string, int > GameVarTypeMap;
	if ( !_init_tables )
	{
		_init_tables = true;

		GameVarTypeMap[ "null"    ] = GV_NULL;
		GameVarTypeMap[ "int"     ] = GV_INT;
		GameVarTypeMap[ "float"   ] = GV_FLOAT;
		GameVarTypeMap[ "string"  ] = GV_STRING;
	}

	for ( XmlReader::XmlReaderIterator propItr = propsItr.NextChild( "Property" );
		propItr.IsValid(); propItr = propItr.NextSibling( "Property" ) )
	{
		propItr.ValidateXMLAttributes( "name,type,value","debug" );

		std::string name = propItr.GetAttributeAsString( "name" );
		std::string type = propItr.GetAttributeAsString( "type" );
		GameVarType gvType = (GameVarType) GetMappedValue( GameVarTypeMap, type, "Unknown GameVar type", GV_NULL );

		name = mName + "." + name;

		GameVar*& gvar = mProperties[ name ];

		if ( !gvar )
		{
			if ( gvType == GV_INT )
			{
				int data = propItr.GetAttributeAsInt( "value" );
				gvar = new GameVar( name.c_str(), gvType, (void*)&data );
			}
			else if ( gvType == GV_FLOAT )
			{
				float data = propItr.GetAttributeAsFloat( "value" );
				gvar = new GameVar( name.c_str(), gvType, (void*)&data );
			}
			else if ( gvType == GV_STRING )
			{
				std::string data = propItr.GetAttributeAsString( "value" );
				gvar = new GameVar( name.c_str(), gvType, (void*)&data );
			}

			//ConsolePrintf( C_FG_LIGHT_PURPLE, "%s: GameVar loaded:\n %s\n"
			//	, mName.c_str()
			//	, gvar->ToString().c_str() );
		}
		else
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: '%s' already defined. Duplicate not loaded. Please make sure your Property names are unique per GameObject.\n"
				, name.c_str() );
			continue;
		}
	}
}
//---------------------------------------
void GameObjectDefinition::LoadControllerDefinition( const XmlReader::XmlReaderIterator& conItr )
{
	ControllerDefinition* contDef = new ControllerDefinition();

	contDef->mName = conItr.GetAttributeAsString( "name", "Cont" );
	contDef->mState = 0U;

	//uint8 stateBit = (uint8) conItr.GetAttributeAsUInt( "state", 1U );
	uint8 stateBit;
	if ( conItr.HasAttribute( "state" ) )
	{
		ConsolePrintf( CONSOLE_WARNING, "Deprecated Attribute: 'Controller.state='int''.\nUse<State name=''> instead.\n" );
		stateBit = (uint8) GetStateIdFromName( conItr.GetAttributeAsString( "state", "1" ) );
	}
	else
	{
		stateBit = (uint8) mActiveStateId;
	}
	
	SetBit( contDef->mState, stateBit );
	std::string contTypeString = conItr.GetAttributeAsString( "type", "AND" );
	contDef->mType = GameController::GetTypeFromString( contTypeString );
	if ( contDef->mType == GameController::GC_EXPR )
	{
		contDef->mExtra = conItr.GetAttributeAsString( "expr" );
	}
	else if ( contDef->mType == GameController::GC_SCRIPT )
	{
		contDef->mExtra = conItr.GetAttributeAsString( "scriptName" );
	}

#if 0
	std::string stateNames;
	for ( int i = 0; i < 32; ++i )
	{
		if ( !mStateNameMapping[ i ].empty() && IsBitSet( contDef->mState, i ) )
			stateNames += ", " + mStateNameMapping[ i ];
	}
	ConsolePrintf( "Created Controller:\n"\
		" Name: %s :"	\
		" Type: %s :"	\
		" State: %s\n"	
		, contDef->mName.c_str()
		, contTypeString.c_str()
		, stateNames.c_str() );
#endif

	for ( XmlReader::XmlReaderIterator trigItr = conItr.NextChild( "AddTrigger" );
		trigItr.IsValid(); trigItr = trigItr.NextSibling( "AddTrigger" ) )
	{
		std::string name = trigItr.GetAttributeAsString( "name" );
		int firstColon = name.find_first_of( ':' );
		if ( firstColon == std::string::npos )
		{
			name = mName + "::" + name;
		}
		contDef->mTriggerNames.push_back( name );
		//ConsolePrintf( " Added Trigger: %s\n", name.c_str() );
	}
	for ( XmlReader::XmlReaderIterator actItr = conItr.NextChild( "AddAction" );
		actItr.IsValid(); actItr = actItr.NextSibling( "AddAction" ) )
	{
		std::string name = actItr.GetAttributeAsString( "name" );
		int firstColon = name.find_first_of( ':' );
		if ( firstColon == std::string::npos )
		{
			name = mName + "::" + name;
		}
		contDef->mActionNames.push_back( name );
		//ConsolePrintf( " Added Action: %s\n", name.c_str() );
	}

	mControllerDefinitions.push_back( contDef );
}
//---------------------------------------
uint32 GameObjectDefinition::GetStateIdFromName( const std::string& name )
{
	uint32 firstFreeSlot = 0, i;
	for ( i = 0; i < 31; ++i )
	{
		if ( !StringUtil::StringiCmp( mStateNameMapping[ i ], name ) )
		{
			firstFreeSlot = i;
			break;
		}

		if ( firstFreeSlot == 0 && mStateNameMapping[ i ].empty() )
		{
			firstFreeSlot = i;
		}
	}

	if ( firstFreeSlot == 0 && i == 31 )
	{
		ConsolePrintf( CONSOLE_ERROR, "Number of per object states exceeded (30).\nNew State blocks will be in global State.\n" );
	}
	else if ( i == 31 )
	{
		mStateNameMapping[ firstFreeSlot ] = name;
	}

	return firstFreeSlot;
}
//---------------------------------------
std::string GameObjectDefinition::GetStateNameFromId( uint32 id ) const
{
	std::string stateNames;
	for ( int i = 0; i < 32; ++i )
	{
		if ( !mStateNameMapping[ i ].empty() && IsBitSet( id, i ) )
		{
			if ( !stateNames.empty() )
			{
				stateNames += ", ";
			}
			stateNames += mStateNameMapping[ i ];
		}
	}
	return stateNames;
}
//---------------------------------------