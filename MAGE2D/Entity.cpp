#include "MageLib.h"

using namespace mage;

std::vector< Entity* > Entity::msEntities;

//---------------------------------------
Entity::Entity( const std::string& name, Sprite* sprite )
	: mName( name )
	, mSprite( sprite )
	, mAlive( true )
	, mAge( 0.0f )
	, mMass( 1.0f )
	, mInvMass( 1.0f )
	, mCollisionRadius( -1.0f )
	, mDamping( 1.0f )
	, mPhysicsFlags( EP_DYNAMIC )
	, mCollisionFlags( EC_ALL )
	, mGameMap( NULL )
{
	mEntityId = msEntities.size();
	RegisterEventCallbacks();
	msEntities.push_back( this );
}
//---------------------------------------
Entity::~Entity()
{
	//ConsolePrintf( C_FG_LIGHT_RED, "Entity %s%u destroyed\n", mName.c_str(), mEntityId );
	if ( mSprite )
	{
		SpriteManager::Manager.DestorySprite( mSprite );
	}
	EventManager::UnregisterObjectForAllEvent( *this );
}
//---------------------------------------
void Entity::OnUpdate( float dt )
{
	if ( mSprite )
	{
		mSprite->Position = mPosition;
	}

	mAge += dt;
}
//---------------------------------------
void Entity::OnUpdatePhysics( float fixedDT )
{
	if ( !mPhysicsFlags ) return;

	if ( mPhysicsFlags & EP_DYNAMIC )
	{
		// Calculate velocity
		mVelocity -= mVelocity * mDamping * fixedDT;
		if ( std::fabs( mVelocity.x ) < 1.0f ) mVelocity.x = 0.0f;

		//if ( std::fabs( mVelocity.x ) > mMaxSpeed ) mVelocity.x = mMaxSpeed;
		//if ( std::fabs( mVelocity.y ) > mMaxSpeed ) mVelocity.y = mMaxSpeed * Mathf::Sign( mVelocity.y );
		//if ( std::fabs( mVelocity.x ) < mMinSpeed ) mVelocity.x = 0.0f;

		// Apply movement
		ApplyMotion( mVelocity * fixedDT );
	}
}
//---------------------------------------
void Entity::OnDraw( SDL_Surface* screen, const Camera& camera )
{
	if ( mSprite )
	{
		mSprite->OnDraw( screen, camera );
	}
}
//---------------------------------------
bool Entity::OnCollision( Entity* other )
{
	//ConsolePrintf( "[%d] %s collided with %s\n", FrameRateControl::GetInstance().GetTotalFrames(), mName.c_str(), other->mName.c_str() );
	//Dictionary params;
	bool result = true;

	//params.Set( "Invoker", this );
	//params.Set( "Instigator", other );
	//params.Set( "Other", other );
	//params.Set( "Return", result );

	//EventManager::FireEvent( "OnEntityCollision", params );
	//EntityDefinition* myDef = EntityDefinition::GetDefinitionByName( mName );
	//myDef->CollisionEvent( "EntityCollision", params );

	for ( auto itr = mCollisionEvents.begin(); itr != mCollisionEvents.end(); ++itr )
	{
		itr->second.FireEvents( this, other );
	}

	//params.Get( "Return", result );

	return result;
}
//---------------------------------------
bool Entity::OnCollision( Tile* tile )
{
	return true;
}
//---------------------------------------
bool Entity::OnCollision( MapObject* mapObject )
{
	return true;
}
//---------------------------------------
void Entity::OnDeath()
{
}
//---------------------------------------
void Entity::ApplyImpulse( const Vec2f& force )
{
	mVelocity += force * mInvMass;
}
//---------------------------------------
void Entity::AddHealth( int health )
{
	mHealth += health;
	Mathi::ClampToRange( mHealth, 0, mMaxHealth );
	if ( mHealth == 0 )
	{
		Kill();
	}
}
//---------------------------------------
void Entity::Kill()
{
	if ( mAlive )
	{
		mAlive = false;
		OnDeath();
	}
}
//---------------------------------------
void Entity::SetPosition( const Vec2f& position )
{
	SetPosition( position.x, position.y );
}
//---------------------------------------
void Entity::SetPosition( float x, float y )
{
	mPosition.Set( x, y );
}
//---------------------------------------
bool Entity::Collides( const Entity& other ) const
{
	RectF A( mPosition.x, mPosition.y, mPosition.x + mCollisionBounds.Right - 1, mPosition.y + mCollisionBounds.Bottom - 1 );
	RectF B( other.mPosition.x, other.mPosition.y, other.mPosition.x + other.mCollisionBounds.Right - 1, other.mPosition.y + other.mCollisionBounds.Bottom - 1 );
	return RectF::TestIntersection( A, B );
}
//---------------------------------------
bool Entity::Collides( float x, float y, float width, float height ) const
{
	RectF A( mPosition.x, mPosition.y, mPosition.x + mCollisionBounds.Right - 1, mPosition.y + mCollisionBounds.Bottom - 1 );
	RectF B( x, y, x + width - 1, y + height - 1 );
	return RectF::TestIntersection( A, B );
}
//---------------------------------------
void Entity::SetAnimationState( const std::string& animName, int frame )
{
	if ( mSprite )
	{
		mSprite->SetAnimationState( animName, frame );
	}
}
//---------------------------------------
void Entity::DestroyAllEntities()
{
	DestroyVector( msEntities );
}
//---------------------------------------
Entity* Entity::GetEntityById( unsigned int id )
{
	if ( id < 0 || id >= msEntities.size() )
	{
		return NULL;
	}
	return msEntities[ id ];
}
//---------------------------------------
void Entity::RemoveDeadEntities()
{
	msEntities.erase( std::remove_if( msEntities.begin(), msEntities.end(), [&]( Entity*& entity ) -> bool
	{
		const bool _ret = !entity->IsAlive();
		if ( _ret )
		{
			delete entity;
		}
		return _ret;
	}), msEntities.end() );
}
//---------------------------------------
void Entity::TriggeredEvent( Dictionary& params )
{
	std::string eventName;
	Sprite* sprite;

	params.Get( "EventName", eventName );

	// Event came from sprite, make sure it is ours
	if ( params.Get( "Sprite", sprite ) == Dictionary::DErr_SUCCESS )
	{
		if ( sprite != mSprite ) return;
	}

	TriggerEvent( eventName );
	//EntityDefinition* myDef = EntityDefinition::GetDefinitionByName( mName );
	//myDef->TriggerEvent( eventName, this );

	//ConsolePrintf( "Entity '%s' (%d) TriggeredEvent: %s\n",
	//	mName.c_str(), mEntityId, eventName.c_str() );
}
//---------------------------------------
void Entity::ApplyMotion( Vec2f velocityStep )
{
	Vec2f deltaPosition;

	deltaPosition.x = velocityStep.x * 0.16f;
	deltaPosition.y = velocityStep.y * 0.16f;

	for ( int i = 0; i < 1000; ++i )
	{
		if ( mCollisionFlags & EC_GHOST || mCollisionFlags == EC_NONE )
		{
			mPosition += deltaPosition;
		}
		else
		{
			// Collision in X
			if ( ValidatePosition( mPosition.x + deltaPosition.x, mPosition.y ) )
			{
				mPosition.x += deltaPosition.x;
			}
			else
			{
				mVelocity.x = 0.0f;
			}

			// Collision in Y
			if ( ValidatePosition( mPosition.x, mPosition.y + deltaPosition.y ) )
			{
				mPosition.y += deltaPosition.y;
			}
			else
			{
				mVelocity.y = 0.0f;
			}
		}

		velocityStep -= deltaPosition;

		// break conditions
		if ( deltaPosition.x > 0 && velocityStep.x <= 0 ) deltaPosition.x = 0;	// Got stopped moving left
		if ( deltaPosition.x < 0 && velocityStep.x >= 0 ) deltaPosition.x = 0;	// Got stopped moving right

		if ( deltaPosition.y > 0 && velocityStep.y <= 0 ) deltaPosition.y = 0;	// Got stopped moving up
		if ( deltaPosition.y < 0 && velocityStep.y >= 0 ) deltaPosition.y = 0;	// Got stopped moving down

		if ( velocityStep.x == 0 ) deltaPosition.x = 0;							// Not moving anymore
		if ( velocityStep.y == 0 ) deltaPosition.y = 0;

		// Break if not moving or no change in position
		if ( velocityStep.x   == 0 && velocityStep.y   == 0 )   break;
		if ( deltaPosition.x  == 0 && deltaPosition.y  == 0 )   break;
	}

	//mPosition += velocityStep;
}
//---------------------------------------
bool Entity::ValidatePosition( float x, float y )
{
	bool posValid = true;

	// Collision check on map
	if ( mGameMap )
	{
		int startX = (int) ( x / mGameMap->GetTileWidth()  );
		int startY = (int) ( y / mGameMap->GetTileHeight() );
		int endX   = (int) ( ( x + mCollisionBounds.Right ) / mGameMap->GetTileWidth()   );
		int endY   = (int) ( ( y + mCollisionBounds.Bottom ) / mGameMap->GetTileHeight() );

		for ( int iy = startY; iy <= endY; ++iy )
		{
			for ( int ix = startX; ix <= endX; ++ix )
			{
				Tile& tile = mGameMap->GetTile( ix, iy, Map::CollisionLayerIndex );
				if ( tile.TileCollision == Tile::TC_SOLID )
				{
					posValid = false;
				}
			}
		}
	}

	// Collision check on sprites
	if ( !( mCollisionFlags & EC_WORLD_ONLY ) )
	{
		for ( auto itr = msEntities.begin(); itr != msEntities.end(); ++itr )
		{
			if ( !ValidatePosition( *itr, x, y ) )
			{
				posValid = false;
			}
		}
	}

	return posValid;
}
//---------------------------------------
bool Entity::ValidatePosition( Entity* entity, float x, float y )
{
	if ( entity            &&				// Valid Entity 
		 entity != this    &&				// Don't collide ourself
		 entity->mCollisionFlags &&			// Has collision
		 entity->IsAlive() &&				// Must be alive
		 entity->Collides( x, y,
			mCollisionBounds.Right,
			mCollisionBounds.Bottom ) )		// We actually collide
	{
		// Collision is with map only, no event should fire
		if ( entity->mCollisionFlags & EC_WORLD_ONLY )
		{
			return true;
		}

		//EntityCollisionInfo::EntityCollisions.push_back( EntityCollisionInfo( this, entity ) );
		EntityCollisionInfo::EntityCollisions.insert( EntityCollisionInfo( this, entity ) );

		// Return false is Entity is solid (not a ghost)
		return entity->mCollisionFlags & EC_GHOST;
	}

	// No collision
	return true;
}
//---------------------------------------
void Entity::RegisterEventCallbacks()
{
	EventManager::RegisterObjectForEvent( "ApplyImpulse", *this, &Entity::EventApplyImpulse );
	EventManager::RegisterObjectForEvent( "SetAnimationState", *this, &Entity::EventSetAnimationState );
	EventManager::RegisterObjectForEvent( "Spawn", *this, &Entity::EventSpawnEntity );
	EventManager::RegisterObjectForEvent( "Kill", *this, &Entity::EventKill );
}
//---------------------------------------
void Entity::TriggerEvent( const std::string& eventName )
{
	auto event = mTriggeredEvents.find( eventName );
	if ( event != mTriggeredEvents.end() )
	{
		event->second.FireEvents( this );
	}
}
//---------------------------------------
void Entity::EventApplyImpulse( Dictionary& params )
{
	Entity* entity;
	Vec2f force;

	if ( params.Get( "Invoker", entity ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_WARNING, "Entity '%s' (%d) : ApplyImpulse : Bad event args for 'invoker'\n",
			mName.c_str(), mEntityId );
		return;
	}

	if ( entity != this ) return;

	if ( params.Get( "Force", force ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_WARNING, "Entity '%s' (%d) : ApplyImpulse : Bad event args for 'force'\n",
			mName.c_str(), mEntityId );
	}

	ApplyImpulse( force );
}
//---------------------------------------
void Entity::EventSetAnimationState( Dictionary& params )
{
	Entity* invoker;
	if ( params.Get( "Invoker", invoker ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_WARNING, "Entity '%s' (%d) : EventSetAnimationState : Bad event args for 'invoker'\n",
			mName.c_str(), mEntityId );
		return;
	}

	if ( invoker != this ) return;

	std::string animName = "Idle";
	int frame = 0;

	params.Get( "AnimationName", animName );
	params.Get( "Frame", frame );

	SetAnimationState( animName, frame );
}
//---------------------------------------
void Entity::EventSpawnEntity( Dictionary& params )
{
	Entity* invoker;
	if ( params.Get( "Invoker", invoker ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_WARNING, "Entity '%s' (%d) : EventSetAnimationState : Bad event args for 'invoker'\n",
			mName.c_str(), mEntityId );
		return;
	}

	if ( invoker != this ) return;

	std::string name;
	std::string animName;

	params.Get( "Name", name );
	params.Get( "AnimationName", animName );

	Dictionary p2;
	p2.Set( "EntityName", name );
	p2.Set( "Position", mPosition );
	EventManager::FireEvent( "SpawnEntity", p2 );
	Entity* spawned=0;
	p2.Get( "Entity", spawned );
	if ( spawned )
	{
		spawned->SetAnimationState( animName, 0 );
	}
}
//---------------------------------------
void Entity::EventKill( Dictionary& params )
{
	Entity* invoker;
	if ( params.Get( "Invoker", invoker ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_WARNING, "Entity '%s' (%d) : EventSetAnimationState : Bad event args for 'invoker'\n",
			mName.c_str(), mEntityId );
		return;
	}

	if ( invoker != this ) return;

	Kill();
}
//---------------------------------------


//---------------------------------------
std::set< EntityCollisionInfo > EntityCollisionInfo::EntityCollisions;
//---------------------------------------
EntityCollisionInfo::EntityCollisionInfo( Entity* A, Entity* B )
	: A( A )
	, B( B )
{}
//---------------------------------------