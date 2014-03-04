/*
 * Author      : Matthew Johnson
 * Date        : 6/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class EntityDefinition
	{
		DEFINE_DEFINITION_BASE_MEMBERS( EntityDefinition, Entity );
	public:
		EntityDefinition( const std::string& entityFile );
		~EntityDefinition();

		// Create an entity at the given position using this definition
		Entity& Create( const Vec2f& position ) const;

		void TriggerEvent( const std::string& eventName, Entity* invoker );

	private:
		static const int DEFAULT_ENTITY_MAX_HEALTH = 100;

		std::string mName;
		std::string mAnimName;
		std::string mUserFlags;
		int mMaxHeath;
		RectI mCollisionBounds;
		int mColliosnFlags;
		int mPhysicsType;
		float mMaxSpeed;
		float mMinSpeed;

		std::vector< std::string > mTriggerEvents;
		std::map< std::string, TriggeredEvent > mTriggeredEvents;
		std::map< std::string, TriggeredEvent > mCollisionEvents;
		std::map< std::string, std::string > mAnimationEvents;
	};

}