/*
 * Author      : Matthew Johnson
 * Date        : 22/May/2013
 * Description :
 *   
 */
 
#pragma once

struct SDL_Surface;

namespace mage
{

	// A component is a collection of named clipping rectangles
	struct SpriteComponent
	{
		std::string Name;
		std::map< std::string, RectI > SpriteClips;
		std::map< std::string, SpriteComponent* > ChildComponents;
	};

	// Collection of named SpriteComponents
	class SpriteDefinition
	{
	public:
		SpriteDefinition();
		~SpriteDefinition();

		std::string Name;
		SDL_Surface* SpriteSheet;
		SpriteComponent* RootComponent;

		SpriteComponent* GetComponentByName( const std::string& name );
		SpriteComponent* GetComponentFromPath( const std::string& path );
	};

}