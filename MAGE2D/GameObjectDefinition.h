/*
 * Author      : Matthew Johnson
 * Date        : 27/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameObjectDefinition
	{
		DEFINE_DEFINITION_BASE_MEMBERS( GameObjectDefinition, GameObject );
		friend class GameNodeEditor;
	public:
		GameObjectDefinition( const std::string& filename );
		~GameObjectDefinition();

		GameObject* Create() const;
		uint32 GetStateIdFromName( const std::string& name );
		std::string GetStateNameFromId( uint32 id ) const;
	private:
		void LoadObject( const XmlReader::XmlReaderIterator& rootItr );
		void LoadInclude( const XmlReader::XmlReaderIterator& inclItr );
		void LoadSpriteData( const XmlReader::XmlReaderIterator& spriteItr );
		void LoadTextData( const XmlReader::XmlReaderIterator& textItr );
		void LoadProperties( const XmlReader::XmlReaderIterator& propsItr );
		void LoadControllerDefinition( const XmlReader::XmlReaderIterator& conItr );

		std::string mName;
		uint32 mInitialState;
		int mPhysicsType;
		int mCollisionType;
		Color mDebugColor;
		bool mDebugDraw;
		RectI mBoundingRect;

		struct SpriteDefinitionInfo
		{
			std::string mAnimName;
			std::string mInitialAnimName;
			bool mSpriteSpace;
			Vec2f mPosition;
		};
		
		std::vector< SpriteDefinitionInfo* > mSpriteDefinitions;

		std::vector< GameString* > mGameStrings;

		struct ControllerDefinition
		{
			std::string mName;
			uint32 mState;
			std::vector< std::string > mTriggerNames;
			std::vector< std::string > mActionNames;
			int mType;
			std::string mExtra;
		};

		uint32 mActiveStateId;
		std::string mStateNameMapping[32];
		std::map< std::string, GameVar* > mProperties;
		std::vector< ControllerDefinition* > mControllerDefinitions;
	};

}