/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class SpriteManager
	{
		SpriteManager();
	public:
		~SpriteManager();

		// Singleton accessor
		static SpriteManager Manager;

		// Load sprite definition file
		bool LoadSpriteDefinition( const char* filename );
		// Load animation info
		bool LoadSpriteAnimations( const char* filename );

		// Returns null if animation doesn't exist
		Sprite* CreateSprite( const std::string& animName, const Vec2f& position,
			const std::string& initialAnimName="Idle" );

		// Update all Sprite animations
		void OnUpdate( float dt );
		// Render all sprites
		void OnDraw( SDL_Surface* screen, const Camera& camera );

		void DestroyAllSprites();
		void DestorySprite( Sprite*& sprite );

	private:
		std::map< std::string, SpriteDefinition > mSpriteDefinitions;
		std::map< std::string, SpriteAnimationSet > mSpriteAnimationSets;
		std::vector< Sprite* > mSprites;

		SpriteComponent* LoadSpriteDefinitionDirectory( SpriteComponent* parent,
			const XmlReader::XmlReaderIterator& itr );
		void DebugPrintSpriteDefinition( const SpriteDefinition& sprDef ) const;
		void DebugPrintSpriteAnimationSet( const SpriteAnimationSet& animSet ) const;
	};

}