#include "MageLib.h"

using namespace mage;


//---------------------------------------
// Static functions
GameObject::ObjectPhysicsType GameObject::GetPhysicsTypeFromString( const std::string& strType )
{
	static bool _init_tables = false;
	static std::map< std::string, int > TypeMap;
	if ( !_init_tables )
	{
		_init_tables = true;

		TypeMap[ "none"      ] = PHYS_NONE;
		TypeMap[ "static"    ] = PHYS_STATIC;
		TypeMap[ "dynamic"   ] = PHYS_DYNAMIC;
	}

	return (ObjectPhysicsType) TypeMap[ strType ];
}
//---------------------------------------
GameObject::ObjectCollisionFlags GameObject::GetCollisionTypeFromString( const std::string& strType )
{
	static bool _init_tables = false;
	static std::map< std::string, int > TypeMap;
	if ( !_init_tables )
	{
		_init_tables = true;

		TypeMap[ "none"         ] = COL_NONE;
		TypeMap[ "ghost"        ] = COL_GHOST;
		TypeMap[ "world"        ] = COL_WORLD_ONLY;
		TypeMap[ "all"          ] = COL_ALL;
	}

	return (ObjectCollisionFlags) TypeMap[ strType ];
}
//---------------------------------------


//---------------------------------------
GameObject::GameObject( const std::string& name,
	const std::map< std::string, GameVar* >& properties,
	uint32 initialState, ObjectPhysicsType physicsType,
	ObjectCollisionFlags collisionType )
	: Name( name )
	, mInitialState( initialState )
	, StateMask( initialState )
	, Visible( true )
	, mCollisionFlags( collisionType )
	, mPhysicsType( physicsType )
	, mDamping( 5.0f )
	, GameMap( NULL )
	, mIsValid( true )
	, mParent( NULL )
{
	SetMass( 1.0f );
	for ( auto itr = properties.begin(); itr != properties.end(); ++itr )
	{
		mProperties[ itr->first ] = itr->second->Copy();
	}
}
//---------------------------------------
GameObject::~GameObject()
{
	DestroyMapByValue( mProperties );
	DestroyVector( mControllers );
	DestroyVector( mTriggers );
	DestroyVector( mActions );
	DestroyVector( mGameStrings );

	for ( auto itr = mSprites.begin(); itr != mSprites.end(); ++itr )
	{
		SpriteManager::Manager.DestorySprite( *itr );
	}

	EventManager::UnregisterObjectForAllEvent( *this );
}
//---------------------------------------
void GameObject::AddController( GameController* controller )
{
	mControllers.push_back( controller );
}
//---------------------------------------
void GameObject::AddTrigger( GameTrigger* trigger )
{
	mTriggers.push_back( trigger );
}
//---------------------------------------
void GameObject::AddAction( GameAction* action )
{
	mActions.push_back( action );
}
//---------------------------------------
void GameObject::ResetStateToInitial()
{
	StateMask = mInitialState;
}
//---------------------------------------
void GameObject::OnUpdate( float dt )
{
	for ( auto itr = mTriggers.begin(); itr != mTriggers.end(); ++itr )
	{
		GameTrigger& trig = **itr;

		trig.OnUpdate( dt );
	}

	for ( auto itr = mControllers.begin(); itr != mControllers.end(); ++itr )
	{
		GameController& cont = **itr;

		// Update controller if it is in the current state
		if ( cont.GetState() == 1 || ( cont.GetState() & StateMask ) )
		{
			cont.OnUpdate();
		}
	}
}
//---------------------------------------
void GameObject::OnUpdatePhysics( float dt )
{
	//mCollidingObjects.clear();

	switch ( mPhysicsType )
	{
		case PHYS_STATIC:
		{
			ApplyMotion( Vec2f::ZERO );
			break;
		}

		case PHYS_DYNAMIC:
		{
			mLinearVelocity -= mLinearVelocity * mDamping * dt;
			ApplyMotion( mLinearVelocity * dt );
			break;
		}

		case PHYS_NONE:
		default:
			break;
	}
}
//---------------------------------------
void GameObject::OnDraw( SDL_Surface* screen, const Camera& camera )
{
	if ( !Visible ) return;

	const int cameraX = (int) camera.GetX();
	const int cameraY = (int) camera.GetY();

	const Vec2f pos = GetPosition();

	const int originX = (int) ( pos.x - cameraX );
	const int originY = (int) ( pos.y - cameraY );

	if ( DebugDraw )
	{
		if ( camera.RectInViewport( BoundingRect ) )
		{
			// Outline
			Surface::DrawRect(
				screen,
				originX,
				originY,
				BoundingRect.Width(),
				BoundingRect.Height(),
				DebugColor.bgra, false );

			// Transparent fill
			Surface::DrawRect(
				screen,
				originX,
				originY,
				BoundingRect.Width(),
				BoundingRect.Height(),
				DebugColor.bgra - 0xaa000000 );

			// Origin
			Surface::DrawRect( screen, originX - 1, originY - 1, 2, 2,
				0xffff0000, true );

			// Center
			Surface::DrawRect( screen,
				originX + BoundingRect.CenterX() - 1,
				originY + BoundingRect.CenterY() - 1,
				2, 2,
				0xff0000ff, true );
		}
	}

	for ( auto itr = mGameStrings.begin(); itr != mGameStrings.end(); ++itr )
	{
		const GameString* gString = *itr;

		std::string outputString = EvaluateExpressionString( gString->Text );

		Vec2i pos = gString->Position;

		// Offset from parent if in world space
		if ( gString->WorldSpace )
		{
			pos += Vec2i( originX, originY );
		}

		Surface::DrawText( screen, pos.x, pos.y,
			gString->TextFont, gString->TextColor, outputString.c_str() );
	}
}
//---------------------------------------
std::string GameObject::EvaluateVariablePropertyString( const std::string& var )
{
	std::string result = var;
	if ( var.length() )
	{
		if ( var.at( 0 ) == '@' && var.length() >= 3 && var.at( 1 ) != '@' )
		{
			GameVar& gvar = GetProperty( var.substr( 1 ) );
			if ( gvar.GetType() == GV_STRING )
			{
				result = EvaluateVariablePropertyString( gvar.ValueString() );
			}
			else
			{
				ConsolePrintf( CONSOLE_WARNING,
					"%s\n"					\
					" objectName = %s\n"	\
					"              ^\n"		\
					" variable must be of type string\n"
					, Name.c_str()
					, var.c_str() );
			}
		}
	}
	return result;
}
//---------------------------------------
bool IsValidVariableChar( const char c )
{
	return ( c >= 'a' && c <= 'z' ) ||
		   ( c >= 'A' && c <= 'Z' ) ||
		   ( c >= '0' && c <= '9' );
}
int GetNextNonVariableCharIndex( const std::string& var, int i )
{
	for ( ; i < var.length(); ++i )
	{
		if ( !IsValidVariableChar( var[i] ) )
		{
			if ( !( i + 1 < var.length() && var[i] == '.' && IsValidVariableChar( var[i+1] ) ) )
				break;
		}
	}
	if ( i == var.length() )
		i = std::string::npos;
	return i;
}
std::string GameObject::EvaluateExpressionString( const std::string& var )
{
	std::string outputString = "";
	int varIndex = -1;
	int nextSpace = 0;
	int lastPos = 0;
	Color lastColor = Color::WHITE;
	do
	{
		varIndex = var.find( '@', varIndex + 1 );
		if ( varIndex != std::string::npos )
		{
			nextSpace = GetNextNonVariableCharIndex( var, varIndex+1 ); //var.find( ' ', varIndex );
			if ( nextSpace == std::string::npos )
				nextSpace = var.length();
			outputString += var.substr( lastPos, varIndex - lastPos );
			lastPos = nextSpace;
			std::string propName = var.substr( varIndex + 1, nextSpace - varIndex - 1 );
			// escape @ sign
			if ( propName.length() && propName.at( 0 ) == '@' )
			{
				varIndex++;
				nextSpace = varIndex;
				lastPos = nextSpace;
			}
			else
			{
				GameVar& gVar = GetProperty( propName );
				//if ( gVar.GetDisplayColor().bgra != lastColor.bgra )
				//{
				//	lastColor = gVar.GetDisplayColor();
				//	char colorHex[24];
				//	sprintf_s( colorHex, "#%X[", lastColor.bgra );
				//	outputString +=colorHex;
				//	outputString += EvaluateVariablePropertyString( gVar.ValueString() );
				//	outputString += "]";
				//
				//}
				//else
					outputString += EvaluateVariablePropertyString( gVar.ValueString() );
			}
		}
		else
		{
			outputString += var.substr( nextSpace );
		}
	} while ( varIndex != std::string::npos );
	return outputString;
}
//---------------------------------------
GameVar& GameObject::GetProperty( const std::string& name )
{
	// Look up ObjectName.PropName
	// or if user specifies a . just use what they give us
	// @TODO clean this up
	int firstDot = name.find_first_of( '.' );
	if ( firstDot == std::string::npos )
	{
		auto found = mProperties.find( Name + '.' + name );
		if ( found != mProperties.end() )
		{
			return *found->second;
		}
		return GameVar::NULL_VAR;
	}

	std::string objectName = name.substr( 0, firstDot );

	if ( objectName == "GameInfo" )
	{
		GameInfo& gi = GameInfo::GetGameInfo();

		auto found = gi.Properties.find( name );
		if ( found != gi.Properties.end() )
		{
			return *found->second;
		}
		return GameVar::NULL_VAR;
	}

	auto found = mProperties.find( name );
	if ( found != mProperties.end() )
	{
		return *found->second;
	}
	return GameVar::NULL_VAR;
}
//---------------------------------------
void GameObject::SetProperty( const std::string& name, const std::string& value )
{
	GameVar& gvar = GetProperty( name );
	if ( gvar == GameVar::NULL_VAR )
	{
		mProperties[ Name + '.' + name ] = new GameVar( name.c_str(), GV_STRING, (void*)&value );
	}
	else
	{
		GameVar::GameVarFromString( gvar, value );
	}
}
//---------------------------------------
bool GameObject::HasProperty( const std::string& name )
{
	return GetProperty( name ) != GameVar::NULL_VAR;
}
//---------------------------------------
void GameObject::ResetAll()
{
	for ( auto itr = mTriggers.begin(); itr != mTriggers.end(); ++itr )
	{
		GameTrigger& trigger = **itr;

		trigger.Reset();
	}

	for ( auto itr = mActions.begin(); itr != mActions.end(); ++itr )
	{
		GameAction& action = **itr;

		action.Reset();
	}
}
//---------------------------------------
void GameObject::ApplyForce( const Vec2f& force )
{
	mLinearVelocity += force * mInvMass;
}
//---------------------------------------
void GameObject::ApplyMovement( const Vec2f& movement )
{
	//Position += movement;
	ApplyMotion( movement );
}
//---------------------------------------
void GameObject::SetLinearVelocity( const Vec2f& velocity )
{
	mLinearVelocity = velocity;
}
//---------------------------------------
void GameObject::ApplyLinearVelocity( const Vec2f& velocity )
{
	mLinearVelocity += velocity;
}
//---------------------------------------
void GameObject::SetMass( float mass )
{
	mMass = mass;
	if ( mass != 0 )
	{
		mInvMass = 1.0f / mass;
	}
	else
	{
		mInvMass = 0;
	}
}
//---------------------------------------
void GameObject::SetBoundsFromSprite()
{
	if ( mSprites.size() )
	{
		Sprite& sprite = *mSprites.front();

		BoundingRect = sprite.GetClippingRectForCurrentAnimation();
	}
}
//---------------------------------------
void GameObject::FinalizeNodes()
{

	for ( auto itr = mTriggers.begin(); itr != mTriggers.end(); ++itr )
	{
		GameTrigger& trigger = **itr;

		trigger.OnCreate();
	}

	for ( auto itr = mActions.begin(); itr != mActions.end(); ++itr )
	{
		GameAction& action = **itr;

		action.OnCreate();
	}
}
//---------------------------------------
void GameObject::ApplyMotion( Vec2f linearVelocityStep )
{
	Vec2f deltaPosition;

	deltaPosition.x = linearVelocityStep.x * 0.16f;
	deltaPosition.y = linearVelocityStep.y * 0.16f;

	for ( int i = 0; i < MAX_PHYSICS_STEPS; ++i )
	{
		// Ghost objects ignore collisions
		if ( mCollisionFlags & COL_GHOST )
		{
			Position += deltaPosition;
		}
		else
		{
			// Collision in X
			if ( ValidatePosition( Position.x + deltaPosition.x, Position.y ) )
			{
				Position.x += deltaPosition.x;
			}
			else
			{
				mLinearVelocity.x = 0.0f;
			}

			// Collision in Y
			if ( ValidatePosition( Position.x, Position.y + deltaPosition.y ) )
			{
				Position.y += deltaPosition.y;
			}
			else
			{
				mLinearVelocity.y = 0.0f;
			}
		}

		linearVelocityStep -= deltaPosition;

		// break conditions
		if ( deltaPosition.x > 0 && linearVelocityStep.x <= 0 ) deltaPosition.x = 0;	// Got stopped moving left
		if ( deltaPosition.x < 0 && linearVelocityStep.x >= 0 ) deltaPosition.x = 0;	// Got stopped moving right

		if ( deltaPosition.y > 0 && linearVelocityStep.y <= 0 ) deltaPosition.y = 0;	// Got stopped moving up
		if ( deltaPosition.y < 0 && linearVelocityStep.y >= 0 ) deltaPosition.y = 0;	// Got stopped moving down

		if ( linearVelocityStep.x == 0 ) deltaPosition.x = 0;							// Not moving anymore
		if ( linearVelocityStep.y == 0 ) deltaPosition.y = 0;

		// Break if not moving or no change in position
		if ( linearVelocityStep.x   == 0 && linearVelocityStep.y   == 0 )   break;
		if ( deltaPosition.x        == 0 && deltaPosition.y        == 0 )   break;
	}
}
//---------------------------------------
bool GameObject::ValidatePosition( float x, float y )
{
	bool posValid = true;

	// Collision check on map
	if ( GameMap )
	{
		int startX = (int) ( x / GameMap->GetTileWidth()  );
		int startY = (int) ( y / GameMap->GetTileHeight() );
		int endX   = (int) ( ( x + BoundingRect.Width() ) / GameMap->GetTileWidth()   );
		int endY   = (int) ( ( y + BoundingRect.Height() ) / GameMap->GetTileHeight() );

		for ( int iy = startY; iy <= endY; ++iy )
		{
			for ( int ix = startX; ix <= endX; ++ix )
			{
				Tile& tile = GameMap->GetTile( ix, iy, Map::CollisionLayerIndex );
				if ( tile.TileCollision == Tile::TC_SOLID )
				{
					posValid = false;
				}
			}
		}
	
		// Collision check on objects
		if ( !( mCollisionFlags & COL_WORLD_ONLY ) )
		{
			const std::vector< GameObject* >& allObjects = GameMap->GetAllObjects();
			for ( auto itr = allObjects.begin(); itr != allObjects.end(); ++itr )
			{
				if ( !ValidatePosition( *itr, x, y ) )
				{
					posValid = false;
				}
			}
		}
	}
	
	return posValid;
}
//---------------------------------------
bool GameObject::ValidatePosition( GameObject* gameObject, float x, float y )
{
	bool isValid = true;
	if ( gameObject            &&				// Validate 
		 gameObject != this    &&				// Don't collide ourself
		 gameObject->mCollisionFlags &&			// Has collision
		 //gameObject->IsAlive() &&				// Must be alive
		 gameObject->Collides( x, y,
		 (float) BoundingRect.Width(),
		 (float) BoundingRect.Height() ) )		// We actually collide
	{
		// Collision is with map only, no event should fire
		if ( (gameObject->mCollisionFlags & COL_WORLD_ONLY) == 0 )
		{
			CollisionInfo::sObjectCollisions.insert( CollisionInfo( this, gameObject ) );
		}
		else
		{
			// Return false if Entity is solid (not a ghost)
			isValid = (gameObject->mCollisionFlags & COL_GHOST) != 0;
		}
	}

	// No collision
	return isValid;
}
//---------------------------------------
bool GameObject::Collides( const GameObject& other ) const
{
	RectF A( Position.x, Position.y, Position.x + BoundingRect.Width(), Position.y + BoundingRect.Height() );
	RectF B( other.Position.x, other.Position.y, other.Position.x + other.BoundingRect.Width(), other.Position.y + BoundingRect.Height() );
	return RectF::TestIntersection( A, B );
}
//---------------------------------------
bool GameObject::Collides( float x, float y, float width, float height ) const
{
	RectF A( Position.x, Position.y, Position.x + BoundingRect.Width(), Position.y + BoundingRect.Height() );
	RectF B( x, y, x + width, y + height );
	return RectF::TestIntersection( A, B );
}
//---------------------------------------
bool GameObject::OnCollision( GameObject* other )
{
	if ( other->IsValid() )
	{
		mCollidingObjects.insert( other );
		return true;
	}
	return false;
}
//---------------------------------------
Sprite* GameObject::GetSpriteByIndex( uint32 index ) const
{
	if ( index < mSprites.size() )
	{
		return mSprites[ index ];
	}
	return NULL;
}
//---------------------------------------
Vec2f GameObject::GetPosition() const
{
	if ( mParent )
	{
		return mParent->GetPosition() + Position;
	}
	return Position;
}
//---------------------------------------
bool GameObject::IsOnSolidGround()
{
	bool onGround = false;
	float yCheck = Position.y;
	if ( Mathf::Sign( GameMap->Gravity.y ) < 0 )
		yCheck += -BoundingRect.Top - 1.0f;
	else
		yCheck += 1.0f;

	if ( !ValidatePosition( Position.x, yCheck ) )
		onGround = true;
	return onGround;
}
//---------------------------------------


//---------------------------------------
// CollisionInfo
//---------------------------------------
std::set< GameObject::CollisionInfo > GameObject::CollisionInfo::sObjectCollisions;
//---------------------------------------
GameObject::CollisionInfo::CollisionInfo( GameObject* A, GameObject* B )
	: A( A )
	, B( B )
{}
//---------------------------------------