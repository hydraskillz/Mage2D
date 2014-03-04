#include "MageLib.h"

using namespace mage;

DECLARE_DEFINITION_BASE_MEMBERS( EntityDefinition, Entity );

//---------------------------------------
EntityDefinition::EntityDefinition( const std::string& entityFile )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PhyicisTagToEnum;
	static std::map< std::string, int > CollisionFlagToEnum;

	// Set up mappings for faster parsing (and easier to read code!)
	if ( !_init_tables )
	{
		_init_tables = true;

		PhyicisTagToEnum[ "none"    ] = Entity::EP_NONE;
		PhyicisTagToEnum[ "static"  ] = Entity::EP_STATIC;
		PhyicisTagToEnum[ "dynamic" ] = Entity::EP_DYNAMIC;
		PhyicisTagToEnum[ "rigid"   ] = Entity::EP_RIGID;

		CollisionFlagToEnum[ "all"        ] = Entity::EC_ALL;
		CollisionFlagToEnum[ "none"       ] = Entity::EC_NONE;
		CollisionFlagToEnum[ "worldOnly"  ] = Entity::EC_WORLD_ONLY;
		CollisionFlagToEnum[ "entityOnly" ] = Entity::EC_ENTITY_ONLY;
		CollisionFlagToEnum[ "ghost"      ] = Entity::EC_GHOST;
	}

	XmlReader reader( entityFile.c_str() );
	XmlReader::XmlReaderIterator entityItr = reader.ReadRoot();

	if ( entityItr.IsValid() )
	{
		entityItr.ValidateXMLAttributes( "name","maxHealth,flags" );
		entityItr.ValidateXMLChildElemnts( "", "Sprite,Collision,Properties,Trigger,OnEvent" );

		mName      = entityItr.GetAttributeAsString( "name" );
		mMaxHeath  = entityItr.GetAttributeAsInt( "maxHealth", DEFAULT_ENTITY_MAX_HEALTH );
		mUserFlags = entityItr.GetAttributeAsString( "flags", "" );

		// Sprite data
		XmlReader::XmlReaderIterator spriteItr = entityItr.NextChild( "Sprite" );
		if ( spriteItr.IsValid() )
		{
			spriteItr.ValidateXMLAttributes( "animationSet","relativeToCamera" );

			std::string basePath = StringUtil::StripFilenameFromPath( entityFile );
			mAnimName = spriteItr.GetAttributeAsString( "animationSet" );
			std::string animPath = basePath + mAnimName;

			SpriteManager::Manager.LoadSpriteAnimations( animPath.c_str() );

			XmlReader::XmlReaderIterator cbItr = spriteItr.NextChild( "AnimationCallback" );
			if ( cbItr.IsValid() )
			{
				cbItr.ValidateXMLAttributes( "animationName", "onComplete" );

				std::string animName = cbItr.GetAttributeAsString( "animationName" );
				std::string onComplete = cbItr.GetAttributeAsString( "onComplete", "" );

				if ( onComplete.length() )
				{
					mAnimationEvents[ animName ] = onComplete;
				}
			}
		}

		// Collision data
		XmlReader::XmlReaderIterator colItr = entityItr.NextChild( "Collision" );
		if ( colItr.IsValid() )
		{
			colItr.ValidateXMLAttributes( "physics,collision,shape","" );
			colItr.ValidateXMLChildElemnts( "","Box,Dynamics" );

			std::string physics   = colItr.GetAttributeAsString( "physics" );
			std::string collision = colItr.GetAttributeAsString( "collision" );
			std::string shape     = colItr.GetAttributeAsString( "shape" );

			mPhysicsType   = GetMappedValue( PhyicisTagToEnum, physics, "Unknown physics type" );
			mColliosnFlags = GetMappedValue( CollisionFlagToEnum, collision, "Unknown collision flags" );

			// Box collision
			if ( shape == "box" )
			{
				XmlReader::XmlReaderIterator boxItr = colItr.NextChild( "Box" );
				if ( boxItr.IsValid() )
				{
					boxItr.ValidateXMLAttributes( "width,height","offsetX,offsetY" );

					int offsetX = boxItr.GetAttributeAsInt( "offsetX", 0 );
					int offsetY = boxItr.GetAttributeAsInt( "offsetY", 0 );
					int width   = boxItr.GetAttributeAsInt( "width" );
					int height  = boxItr.GetAttributeAsInt( "height" );

					mCollisionBounds.Set( offsetX, offsetY, width, height );
				}
				else
				{
					ConsolePrintf( CONSOLE_WARNING, "Warning: box collision shape specified but not declared.\n" );
				}
			}
			else
			{
				ConsolePrintf( CONSOLE_WARNING, "Warning: Unknown collision shape '%s'\n", shape.c_str() );
			}

			// Dynamics properties
			if ( mPhysicsType & Entity::EP_DYNAMIC )
			{
				XmlReader::XmlReaderIterator phyItr = colItr.NextChild( "Dynamics" );
				if ( phyItr.IsValid() )
				{
					phyItr.ValidateXMLAttributes( "", "minSpeed,maxSpeed" );

					mMinSpeed = phyItr.GetAttributeAsFloat( "minSpeed", -10000 );
					mMaxSpeed = phyItr.GetAttributeAsFloat( "maxSpeed",  10000 );
				}
			}
		}

		// Triggers
		for ( XmlReader::XmlReaderIterator trigItr = entityItr.NextChild( "Trigger" );
			  trigItr.IsValid(); trigItr = trigItr.NextSibling( "Trigger" ) )
		{
			trigItr.ValidateXMLAttributes( "type", "name,flags,count" );

			std::string name = trigItr.GetAttributeAsString( "name", "" );
			std::string type = trigItr.GetAttributeAsString( "type" );
			int count = trigItr.GetAttributeAsInt( "count", 0 );
			std::string flags = trigItr.GetAttributeAsString( "flags", "" );

			TriggeredEvent triggeredEvent;
			triggeredEvent.SetTriggerCount( count );
			triggeredEvent.SetTriggerFlags( flags );

			for ( XmlReader::XmlReaderIterator eventItr = trigItr.NextChild();
				  eventItr.IsValid(); eventItr = eventItr.NextSibling() )
			{
				Dictionary event;
				Dictionary params;
				std::string eventName = eventItr.ElementName();

				// ApplyImpulse
				if ( eventItr.ElementNameEquals( "ApplyImpulse" ) )
				{
					eventItr.ValidateXMLAttributes( "force", "" );

					Vec2f force = eventItr.GetAttributeAsVec2f( "force" );

					params.Set( "Force", force );
				}
				// Interact
				else if ( entityItr.ElementNameEquals( "OnUse" ) )
				{
					eventItr.ValidateXMLAttributes( "", "" );
				}
				// Set Animation State
				else if ( eventName == "SetAnimationState" )
				{
					eventItr.ValidateXMLAttributes( "animationName", "frame" );

					std::string animName = eventItr.GetAttributeAsString( "animationName" );
					int frame = eventItr.GetAttributeAsInt( "frame", 0 );

					params.Set( "AnimationName", animName );
					params.Set( "Frame", frame );
				}
				// Spawn Entity
				else if ( eventName == "Spawn" )
				{
					eventItr.ValidateXMLAttributes( "name", "animationState" );

					std::string name = eventItr.GetAttributeAsString( "name" );
					std::string animName = eventItr.GetAttributeAsString( "animationState", "Idle" );

					params.Set( "Name", name );
					params.Set( "AnimationName", animName );
				}
				// Kill
				else if ( eventName == "Kill" )
				{
					eventItr.ValidateXMLAttributes( "", "" );
				}

				event.Set( "EventName", eventName );
				event.Set( "Args", params );

				triggeredEvent.AddEvent( event );
			}

			if ( type == "event" )
			{
				mTriggeredEvents.insert( std::make_pair( name, triggeredEvent ) );
				mTriggerEvents.push_back( name );
			}
			else if ( type == "collision" )
			{
				mCollisionEvents.insert( std::make_pair( name, triggeredEvent ) );
			}
		}

		ConsolePrintf( "Loaded Entity '%s'.\n", mName.c_str() );

		RegisterDefinition( mName );
	}
}
//---------------------------------------
EntityDefinition::~EntityDefinition()
{}
//---------------------------------------
Entity& EntityDefinition::Create( const Vec2f& position ) const
{
	Entity* entity = new Entity( mName, NULL );

	entity->mPosition = position;
	entity->mSprite = SpriteManager::Manager.CreateSprite( mAnimName, position );
	entity->mCollisionBounds = mCollisionBounds;
	entity->mCollisionFlags = mColliosnFlags;
	entity->mPhysicsFlags = mPhysicsType;
	entity->mMaxSpeed = mMaxSpeed;
	entity->mMinSpeed = mMinSpeed;
	entity->mCollisionEvents = mCollisionEvents;
	entity->mTriggeredEvents = mTriggeredEvents;
	entity->SetUserFlags( mUserFlags );

	// Setup event listeners
	for ( auto itr = mTriggerEvents.begin(); itr != mTriggerEvents.end(); ++itr )
	{
		EventManager::RegisterObjectForEvent( *itr,
			*entity, &Entity::TriggeredEvent );
	}

	// Set animation event listeners
	if ( entity->mSprite )
	{
		for ( auto itr = mAnimationEvents.begin(); itr != mAnimationEvents.end(); ++itr )
		{
			entity->mSprite->RegisterOnCompletionEvent( itr->first, itr->second );
		}
	}

	return *entity;
}
//---------------------------------------
void EntityDefinition::TriggerEvent( const std::string& eventName, Entity* invoker )
{
	auto event = mTriggeredEvents.find( eventName );
	if ( event != mTriggeredEvents.end() )
	{
		event->second.FireEvents( invoker );
	}
}
//---------------------------------------