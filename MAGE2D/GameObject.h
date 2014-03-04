/*
 * Author      : Matthew Johnson
 * Date        : 26/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameObject
	{
		friend class GameObjectDefinition;
		friend class Map;
	public:

		enum ObjectPhysicsType
		{
			PHYS_NONE,
			PHYS_STATIC,
			PHYS_DYNAMIC
		};

		enum ObjectCollisionFlags
		{
			COL_NONE           = 0x0000,
			COL_GHOST          = 0x0001,
			COL_WORLD_ONLY     = 0x0002,
			COL_ALL            = 0x0004
		};

		static const int MAX_PHYSICS_STEPS = 100;

		static ObjectPhysicsType GetPhysicsTypeFromString( const std::string& strType );
		static ObjectCollisionFlags GetCollisionTypeFromString( const std::string& strType );

		GameObject( const std::string& name,
			const std::map< std::string, GameVar* >& properties,
			uint32 initialState=2U, ObjectPhysicsType physicsType=PHYS_STATIC,
			ObjectCollisionFlags collisionType=COL_ALL );
		virtual ~GameObject();

		void AddController( GameController* controller );
		void AddTrigger( GameTrigger* trigger );
		void AddAction( GameAction* action );
		void ResetStateToInitial();
		void OnUpdate( float dt );
		void OnUpdatePhysics( float dt );
		void OnDraw( SDL_Surface* screen, const Camera& camera );
		GameVar& GetProperty( const std::string& name );
		void SetProperty( const std::string& name, const std::string& value );
		bool HasProperty( const std::string& name );
		void ResetAll();
		void FinalizeNodes();

		void ApplyForce( const Vec2f& force );
		void ApplyMovement( const Vec2f& movement );
		void SetLinearVelocity( const Vec2f& velocity );
		void ApplyLinearVelocity( const Vec2f& velocity );

		// Set mass to 0 for infinite mass (unmovable by external forces)
		void SetMass( float mass );
		float GetMass() const { return mMass; }

		// Use primary (first) Sprite's first frame to compute collision bounds
		void SetBoundsFromSprite();

		// List of objects that collided with us last frame
		const std::set< GameObject* >& GetCollidingObjects() const { return mCollidingObjects; }

		bool Collides( const GameObject& other ) const;
		bool Collides( float x, float y, float width, float height ) const;

		bool OnCollision( GameObject* other );

		bool IsValid() const { return mIsValid; }
		void Invalidate() { mIsValid = false; }

		Sprite* GetSpriteByIndex( uint32 index ) const;

		std::string EvaluateVariablePropertyString( const std::string& var );
		std::string EvaluateExpressionString( const std::string& var );

		void SetPhysics( ObjectPhysicsType type ) { mPhysicsType = type; }

		void SetParent( GameObject* parent ) { mParent = parent; }
		Vec2f GetPosition() const;

		// True if the tiles below us (direction of gravity) are solid
		bool IsOnSolidGround();

	protected:
		void ApplyMotion( Vec2f linearVelocityStep );
		bool ValidatePosition( float x, float y );
		bool ValidatePosition( GameObject* gameObject, float x, float y );

	protected:
		std::map< std::string, GameVar* > mProperties;
		std::vector< GameController* > mControllers;
		std::vector< GameTrigger* > mTriggers;
		std::vector< GameAction* > mActions;
		uint32 mInitialState;
		std::vector< Sprite* > mSprites;
		std::vector< GameString* > mGameStrings;
		ObjectPhysicsType mPhysicsType;
		ObjectCollisionFlags mCollisionFlags;
		Vec2f mLinearVelocity;
		float mDamping;
		GameObject* mParent;
		std::set< GameObject* > mCollidingObjects;

	private:
		float mMass;
		float mInvMass;
		bool mIsValid;

	public:
		const std::string Name;
		uint32 StateMask;
		bool Visible;
		Vec2f Position;
		RectI BoundingRect;
		Color DebugColor;
		bool DebugDraw;
		uint32 ObjectId;
		Map* GameMap;

		class CollisionInfo
		{
		public:
			CollisionInfo( GameObject* A, GameObject* B );

			GameObject* A;
			GameObject* B;

			bool operator< ( const CollisionInfo& other ) const
			{
				if ( other.A != A )
				{
					return other.A->ObjectId < A->ObjectId;
				}
				if ( other.B != B )
				{
					return other.B->ObjectId < B->ObjectId;
				}
				return false;
			}

			static std::set< CollisionInfo > sObjectCollisions;
		};
	};

}