#include "MageLib.h"

using namespace mage;

std::map< std::string, MapObject* > MapObject::msMapObjects;

//---------------------------------------
MapObject::MapObject( const std::string& name, int type,
	const RectI& rect, bool visible )
	: Name( name )
	, Type( (ObjectType) type )
	, BoundingRect( rect )
	, Visible( visible )
	, Origin( rect.Left, rect.Top )
{
	msMapObjects[ name ] = this;
}
//---------------------------------------
MapObject::~MapObject()
{
	EventManager::UnregisterObjectForAllEvent( *this );
}
//---------------------------------------
void MapObject::OnDraw( SDL_Surface* screen )
{
	if ( Visible )
	{
		// @TODO
	}
}
//---------------------------------------
int MapObject::DecodeTypeString( const std::string& str )
{
	if ( str == "Spawn" ) return Obj_SPAWN;
	if ( str == "Trigger" ) return Obj_TRIGGER;
	if ( str == "CameraBounds" ) return Obj_CAMERA_BOUNDS;
	return Obj_NONE;
}
//---------------------------------------
void MapObject::EvalProperties()
{
	// Look for the following keys:
	// TriggerOn : event to trigger stuff on
	std::string triggerName;
	if ( Properties.Get( "TriggerOn", triggerName ) == Dictionary::DErr_SUCCESS )
	{
		EventManager::RegisterObjectForEvent( triggerName, *this, &MapObject::OnTriggered );
	}
	// EntityFlags : colliding entity must have these flags to cause event to fire
	std::string entityFlags;
	if ( Properties.Get( "Entity", entityFlags ) == Dictionary::DErr_SUCCESS )
	{
		StringUtil::Tokenize( entityFlags, mEntityFlags, "," );
	}
	// TeleportToObject : teleports colliding entity to object position
	Properties.Get( "TeleportToObject", mTeleportTargetName );
	// FireEvent : event to fire when this object is triggered
	Properties.Get( "FireEvent", mFireEventName );
	
	// Do obj specific stuff...
	// 
}
//---------------------------------------
// Event call backs
//---------------------------------------
void MapObject::OnTriggered( Dictionary& params )
{
	// Spawners spawn stuff when triggered
	if ( Type == Obj_SPAWN )
	{
		//ConsolePrintf( C_FG_LIGHT_PURPLE, "Spawner '%s' was triggered!\n", Name.c_str() );

		std::string spawnable;
		if ( Properties.Get( "Spawn", spawnable ) == Dictionary::DErr_SUCCESS )
		{
			ConsolePrintf( C_FG_LIGHT_PURPLE, "Spawning '%s'\n", spawnable.c_str() );
			Dictionary spawnerParams;
			spawnerParams.Set( "EntityName", spawnable );
			spawnerParams.Set( "Position", (Vec2f) Origin );
			EventManager::FireEvent( "SpawnEntity", spawnerParams );
		}
	}
	// Triggers fire off other events
	else if ( Type == Obj_TRIGGER )
	{
		Entity* entity;
		params.Get( "Invoker", entity );

		// This trigger teleports entities
		if ( mTeleportTargetName.size() )
		{
			if ( BoundingRect.Contains( (int) entity->GetPosition().x, (int) entity->GetPosition().y ) )
			{
				MapObject* target = msMapObjects[ mTeleportTargetName ];
				if ( target )
				{
					entity->SetPosition( (Vec2f) target->Origin );
				}
			}
			else return;
		}

		ConsolePrintf( C_FG_YELLOW, "Trigger '%s' was triggered!\n", Name.c_str() );
	}
	// Camera Bounds limit the cameras movement
	else if ( Type == Obj_CAMERA_BOUNDS )
	{
		Dictionary params2;
		params2.Set( "Bounds", BoundingRect );
		EventManager::FireEvent( "SetCameraBounds", params2 );
		ConsolePrintf( C_FG_GREEN, "CameraBounds set to '%s'\n", Name.c_str() );
	}

	// Now do general object stuff

	// Fire an event if we are set to
	if ( mFireEventName.length() )
	{
		EventManager::FireEvent( mFireEventName );
	}
}
//---------------------------------------
void MapObject::DestroyAllObjects()
{
	msMapObjects.clear();
}
//---------------------------------------