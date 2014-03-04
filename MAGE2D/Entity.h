/*
 * Author      : Matthew Johnson
 * Date        : 5/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class Entity
	{
	public:
		friend class EntityDefinition;
		friend class Map;

		enum EntityCollisionType
		{
			EC_NONE				   ,	// No collision detection/response
			EC_WORLD_ONLY  = 0x0001,	// Entity to Tile only
			EC_ENTITY_ONLY = 0x0002,	// Entity to Entity only
			EC_ALL         = 0x0004,	// Entity to Tile and Entity
			EC_GHOST       = 0x0008,	// No collision response, but events are generated
		};

		enum EntityPhysicsType
		{
			EP_NONE			   ,	// No physics will be evaluated
			EP_STATIC  = 0x0001,	// Entity is not effected by external forces
			EP_DYNAMIC = 0x0002,	// Entity is effected by linear forces
			EP_RIGID   = 0x0004		// Entity is effected by Linear and rotational forces
		};

		Entity( const std::string& name, Sprite* sprite );
		virtual ~Entity();

		virtual void OnUpdate( float dt );
		virtual void OnUpdatePhysics( float fixedDT );
		virtual void OnDraw( SDL_Surface* screen, const Camera& camera );

		virtual bool OnCollision( Entity* other );
		virtual bool OnCollision( Tile* tile );
		virtual bool OnCollision( MapObject* mapObject );

		virtual void OnDeath();

		void ApplyImpulse( const Vec2f& force );

		inline bool IsAlive() const;
		inline int GetHealth() const;
		inline float GetAge() const;
		inline unsigned int GetEntityId() const;

		inline void SetMaxHealth( int maxHealth );
		inline void SetHealthToMax();

		void AddHealth( int health );
		void Kill();

		inline Vec2f GetPosition() const;
		void SetPosition( const Vec2f& position );
		void SetPosition( float x, float y );

		bool Collides( const Entity& other ) const;
		bool Collides( float x, float y, float width, float height ) const;

		inline float GetMass() const;
		inline void SetMass( float mass );

		inline void SetGameMap( Map* map );

		void SetAnimationState( const std::string& animName, int frame=0 );

		inline void SetUserFlags( const std::string& flags );
		inline const std::string& GetUserFlags() const;

		template< typename TFunction >
		static void ForEachEntity( TFunction fn );

		static void DestroyAllEntities();
		static Entity* GetEntityById( unsigned int id );
		static void RemoveDeadEntities();

		// Events
		void TriggeredEvent( Dictionary& params );
		void EventApplyImpulse( Dictionary& params );
		void EventSetAnimationState( Dictionary& params );
		void EventSpawnEntity( Dictionary& params );
		void EventKill( Dictionary& params );

	protected:
		void ApplyMotion( Vec2f velocityStep );
		bool ValidatePosition( float x, float y );
		bool ValidatePosition( Entity* entity, float x, float y );
		void RegisterEventCallbacks();
		void TriggerEvent( const std::string& eventName );

		// Entity name
		std::string mName;
		unsigned int mEntityId;
		std::string mUserFlags;

		// Game map this Entity is on
		Map* mGameMap;

		// State
		bool mAlive;
		int mHealth;		// Health of the Entity. If <= 0 Entity is killed.
		int mMaxHealth;		// Maximum amount of health the Entity can have.
		float mAge;			// Time since we spawned, in seconds

		// Physics properties
		int mPhysicsFlags;
		Vec2f mPosition;
		Vec2f mVelocity;
		Vec2f mAcceleration;
		float mDamping;
		float mMass;
		float mInvMass;
		float mMaxSpeed;
		float mMinSpeed;

		// Collision properties
		int mCollisionFlags;
		RectI mCollisionBounds;
		float mCollisionRadius;

		// Display properties
		Sprite* mSprite;

		// Events
		std::map< std::string, mage::TriggeredEvent > mTriggeredEvents;
		std::map< std::string, mage::TriggeredEvent > mCollisionEvents;

	private:
		static std::vector< Entity* > msEntities;
	};

	class EntityCollisionInfo
	{
	public:
		EntityCollisionInfo( Entity* A, Entity* B );

		Entity* A;
		Entity* B;

		bool operator< ( const EntityCollisionInfo& other ) const
		{
			if ( other.A == A && other.B == B )
			{
				return false;
			}
			return other.A->GetEntityId() < A->GetEntityId();
		}

		static std::set< EntityCollisionInfo > EntityCollisions;
	};

#include "Entity.hpp"
}