#include "MageLib.h"

#include <SDL.h>

using namespace mage;

//---------------------------------------
SpriteDefinition::SpriteDefinition()
	: SpriteSheet( NULL )
	, RootComponent( nullptr )
{}
//---------------------------------------
SpriteDefinition::~SpriteDefinition()
{
	// @TODO put in terminator callback
	//SDL_FreeSurface( SpriteSheet );
}
//---------------------------------------
// Private recursive helper for GetComponentByName()
SpriteComponent* _GetComponentByName( SpriteComponent* sprComp, const std::string& name )
{
	if ( sprComp )
	{
		// Search our children
		auto found = sprComp->ChildComponents.find( name );
		if ( found != sprComp->ChildComponents.end() )
		{
			return sprComp;
		}
		// Search our children's children
		else
		{
			for ( auto itr = sprComp->ChildComponents.begin();
				itr != sprComp->ChildComponents.end(); ++itr )
			{
				SpriteComponent* foundInChildren = _GetComponentByName( itr->second, name );
				// Found in children
				if ( foundInChildren )
				{
					return foundInChildren;
				}
			}
		}
	}

	// Not found
	return nullptr;
}
SpriteComponent* SpriteDefinition::GetComponentByName( const std::string& name )
{
	return _GetComponentByName( RootComponent, name );
}
//---------------------------------------
SpriteComponent* SpriteDefinition::GetComponentFromPath( const std::string& path )
{
	std::vector< std::string > pathTokens;
	StringUtil::Tokenize( path, pathTokens, "/" );

	SpriteComponent* sprComp = RootComponent;
	for ( auto itr = pathTokens.begin(); itr != pathTokens.end(); ++itr )
	{
		sprComp = sprComp->ChildComponents[ *itr ];
	}

	return sprComp;
}
//---------------------------------------