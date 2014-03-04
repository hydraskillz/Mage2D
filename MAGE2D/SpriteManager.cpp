#include "MageLib.h"

using namespace mage;

SpriteManager SpriteManager::Manager;

//---------------------------------------
SpriteManager::SpriteManager()
{}
//---------------------------------------
SpriteManager::~SpriteManager()
{}
//---------------------------------------
bool SpriteManager::LoadSpriteDefinition( const char* filename )
{
	/* Definition file layout
	<img name"image.png" w="" h="" >
		<definitions>
			<dir name="" >
				<spr name="id" x="" y="" w="" h="" />
				...
			</dir>
			...
		</definitions>
	<img>
	*/

	SpriteDefinition sprDef;

	sprDef.Name = StringUtil::ExtractFilenameFromPath( filename );

	XmlReader reader( filename );
	XmlReader::XmlReaderIterator root = reader.ReadRoot();

	root.ValidateXMLAttributes( "name,w,h","" );
	root.ValidateXMLChildElemnts( "definitions","" );

	// Load spritesheet image
	char* buffer=0;
	unsigned int size;
	std::string spriteSheetName = std::string( filename ).substr( 0, std::string( filename ).find_last_of( "/" ) + 1 )
		+ root.GetAttributeAsString( "name" );
	if ( OpenDataFile( spriteSheetName.c_str(), buffer, size ) != FSE_NO_ERROR )
	{
		ConsolePrintf( CONSOLE_WARNING, "Failed to load spritesheet '%s'\n", spriteSheetName.c_str() );
		return false;
	}
	//sprDef.SpriteSheet = Surface::LoadSurface( 
	//	( std::string( filename ).substr( 0, std::string( filename ).find_last_of( "/" ) + 1 )
	//	+ root.GetAttributeAsString( "name" ) ).c_str() );
	sprDef.SpriteSheet = Surface::LoadSurfaceMem( buffer, size, 1, true );

	// Loop through the definitions
	for ( XmlReader::XmlReaderIterator defItr = root.NextChild( "definitions" ).NextChild( "dir" );
		defItr.IsValid(); defItr = defItr.NextSibling( "dir" ) )
	{
		sprDef.RootComponent = LoadSpriteDefinitionDirectory( nullptr, defItr );
	}

	mSpriteDefinitions[ sprDef.Name ] = sprDef;

	//DebugPrintSpriteDefinition( sprDef );

	return true;
}
//---------------------------------------
SpriteComponent* SpriteManager::LoadSpriteDefinitionDirectory( SpriteComponent* parent,
	const XmlReader::XmlReaderIterator& defItr )
{
	defItr.ValidateXMLAttributes( "name","" );
	defItr.ValidateXMLChildElemnts( "","spr,dir" );

	SpriteComponent* sprComp = New0 SpriteComponent();

	sprComp->Name = defItr.GetAttributeAsString( "name" );

	// Root of sprite tree is simply a slash
	if ( sprComp->Name == "/" ) sprComp->Name = "root";

	// Get the clipping rectangles
	for ( XmlReader::XmlReaderIterator sprItr = defItr.NextChild( "spr" );
		sprItr.IsValid(); sprItr = sprItr.NextSibling( "spr" ) )
	{
		sprItr.ValidateXMLAttributes( "name,x,y,w,h","" );
		sprItr.ValidateXMLChildElemnts( "","" );

		std::string sprName = sprItr.GetAttributeAsString( "name" );
		int x = sprItr.GetAttributeAsInt( "x" );
		int y = sprItr.GetAttributeAsInt( "y" );
		int w = sprItr.GetAttributeAsInt( "w" );
		int h = sprItr.GetAttributeAsInt( "h" );

		sprComp->SpriteClips[ sprName ] = RectI( x, y, x + w, y + h );
	}

	if ( parent )
	{
		parent->ChildComponents[ sprComp->Name ] = sprComp;
	}

	// Get child definitions
	for ( XmlReader::XmlReaderIterator defCItr = defItr.NextChild( "dir" );
		defCItr.IsValid(); defCItr = defCItr.NextSibling( "dir" ) )
	{
		LoadSpriteDefinitionDirectory( sprComp, defCItr );
	}

	return sprComp;
}
//---------------------------------------
bool SpriteManager::LoadSpriteAnimations( const char* filename )
{
	/* Animation file
	<animations spriteSheet="img.png" ver="1.2">
		<anim name="" loops="">
			<cell index="0" delay="">
				<spr name="path_to_sprite_component" x="" y="" z=""/>
				...
			</cell>
			...
		</anm>
		...
	</animations>
	*/

	SpriteAnimationSet animSet;

	animSet.Name = StringUtil::ExtractFilenameFromPath( filename );

	// Already loaded
	if ( mSpriteAnimationSets.find( animSet.Name ) != mSpriteAnimationSets.end() )
	{
		return true;
	}

	XmlReader reader( filename );
	XmlReader::XmlReaderIterator root = reader.ReadRoot();

	if ( !root.IsValid() )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to load %s\n", filename );
		return false;
	}

	root.ValidateXMLAttributes( "spriteSheet,ver","" );
	root.ValidateXMLChildElemnts( "anim","" );

	// Get the name of the sprite definition this animation set is for
	std::string definitionName = root.GetAttributeAsString( "spriteSheet" );
	auto foundDef = mSpriteDefinitions.find( definitionName );
	if ( foundDef != mSpriteDefinitions.end() )
	{
		animSet.MyDefinition = &foundDef->second;
	}
	else
	{
		// Definition not loaded - try to load it
		std::string basePath = StringUtil::StripFilenameFromPath( filename );
		std::string sprdefPath = basePath + definitionName;
		if ( LoadSpriteDefinition( sprdefPath.c_str() ) )
		{
			animSet.MyDefinition = &mSpriteDefinitions[ definitionName ];
		}
		// Failure
		else
		{
			assertion( false, "Failed to link animation '%s' to sprite '%s'\nSprite file not found.\n",
				animSet.Name.c_str(), definitionName.c_str() );
			return false;
		}
	}

	// Loop through the animations
	for ( XmlReader::XmlReaderIterator animItr = root.NextChild( "anim" );
		animItr.IsValid(); animItr = animItr.NextSibling( "anim" ) )
	{
		animItr.ValidateXMLAttributes( "name,loops","" );
		animItr.ValidateXMLChildElemnts( "cell","" );

		SpriteAnimation sprAnim;

		sprAnim.Name = animItr.GetAttributeAsString( "name" );
		sprAnim.LoopType = animItr.GetAttributeAsInt( "loops" );

		// Loop through all the frames
		for ( XmlReader::XmlReaderIterator cellItr = animItr.NextChild( "cell" );
			cellItr.IsValid(); cellItr = cellItr.NextSibling( "cell" ) )
		{
			cellItr.ValidateXMLAttributes( "index,delay","" );
			cellItr.ValidateXMLChildElemnts( "spr","" );

			SpriteAnimationFrame animFrame;

			animFrame.Index = cellItr.GetAttributeAsInt( "index" );
			animFrame.Delay = cellItr.GetAttributeAsInt( "delay" );

			// Get the frame components
			for ( XmlReader::XmlReaderIterator sprItr = cellItr.NextChild( "spr" );
				sprItr.IsValid(); sprItr = sprItr.NextSibling( "spr" ) )
			{
				sprItr.ValidateXMLAttributes( "name,x,y,z","" );
				sprItr.ValidateXMLChildElemnts( "","" );

				SpriteAnimationComponent* animComp = New0 SpriteAnimationComponent();

				std::string componentName = sprItr.GetAttributeAsString( "name" );

				animComp->FrameOffsetX = sprItr.GetAttributeAsInt( "x" );
				animComp->FrameOffsetY = sprItr.GetAttributeAsInt( "y" );
				animComp->FrameZOrder  = sprItr.GetAttributeAsInt( "z" );

				// Get the component this animation references
				std::string clipName = StringUtil::ExtractFilenameFromPath( componentName );
				std::string clipPath = componentName.substr( 1, componentName.find_last_of( "/" )-1 );
				animComp->SprComponent = 
					animSet.MyDefinition->
					GetComponentFromPath( clipPath );
				animComp->SprComponentClipName = clipName;

				animFrame.SprAnimComponent = animComp;

				// @TODO Load all components
				break;
			}

			sprAnim.Frames.push_back( animFrame );
		}

		sprAnim.FrameCount = (int) sprAnim.Frames.size();
		animSet.Animations[ sprAnim.Name ] = sprAnim;
	}

	mSpriteAnimationSets[ animSet.Name ] = animSet;

	//DebugPrintSpriteAnimationSet( animSet );

	return true;
}
//---------------------------------------
Sprite* SpriteManager::CreateSprite( const std::string& animName, const Vec2f& position,
	const std::string& initialAnimName )
{
	Sprite* sprite = new Sprite( mSpriteAnimationSets[ animName ], initialAnimName );

	sprite->Position = position;

	mSprites.push_back( sprite );

	return sprite;
}
//---------------------------------------
void SpriteManager::OnUpdate( float dt )
{
	for ( auto itr = mSprites.begin(); itr != mSprites.end(); ++itr )
	{
		(*itr)->OnUpdate( dt );
	}
}
//---------------------------------------
void SpriteManager::OnDraw( SDL_Surface* screen, const Camera& camera )
{
	for ( auto itr = mSprites.begin(); itr != mSprites.end(); ++itr )
	{
		(*itr)->OnDraw( screen, camera );
	}
}
//---------------------------------------
void SpriteManager::DestroyAllSprites()
{
	DestroyVector( mSprites );
}
//---------------------------------------
void SpriteManager::DestorySprite( Sprite*& sprite )
{
	auto foundItr = std::find( mSprites.begin(), mSprites.end(), sprite );
	if ( foundItr != mSprites.end() )
	{
		mSprites.erase(
			foundItr, foundItr + 1 );
		Delete0( sprite );
	}
}
//---------------------------------------
// Private recursive helper for DebugPrintSpriteDefinition()
void _DebugPrintSpriteDefinition( SpriteComponent* sprComp )
{
	if ( !sprComp ) return;

	ConsolePrintf( " Sprite Component: '%s'\n", sprComp->Name.c_str() );
	for ( auto jtr = sprComp->SpriteClips.begin();
		jtr != sprComp->SpriteClips.end(); ++jtr )
	{
		const RectI& r = jtr->second;

		ConsolePrintf( "  Clip: '%s'\n   x = %d\n   y = %d\n   w = %d\n   h = %d\n",
			jtr->first.c_str(), r.Left, r.Right, r.Width(), r.Height() );
	}

	for ( auto itr = sprComp->ChildComponents.begin();
		itr != sprComp->ChildComponents.end(); ++itr )
	{
		_DebugPrintSpriteDefinition( itr->second );
	}
}
//---------------------------------------
void SpriteManager::DebugPrintSpriteDefinition( const SpriteDefinition& sprDef ) const
{
	ConsolePrintf( "Sprite definition for '%s'\n", sprDef.Name.c_str() );
	SpriteComponent* sprComp = sprDef.RootComponent;
	_DebugPrintSpriteDefinition( sprComp );
}
//---------------------------------------
void SpriteManager::DebugPrintSpriteAnimationSet( const SpriteAnimationSet& animSet ) const
{
	ConsolePrintf( "Animation Set: '%s'\n", animSet.Name.c_str() );
	ConsolePrintf( " Uses definition: '%s'\n", animSet.MyDefinition->Name.c_str() );
	for ( auto itr = animSet.Animations.begin(); 
		itr != animSet.Animations.end(); ++itr )
	{
		const SpriteAnimation& sprAnim = itr->second;

		ConsolePrintf( " Animation: '%s'\n", sprAnim.Name.c_str() );
		ConsolePrintf( " LoopType: %d\n", sprAnim.LoopType );
		for ( size_t i = 0; i < sprAnim.Frames.size(); ++i )
		{
			ConsolePrintf( "  Frame %d\n", i );

			const SpriteAnimationFrame& sprFrame = sprAnim.Frames[ i ];

			ConsolePrintf( "    Index: %d\n", sprFrame.Index );
			ConsolePrintf( "    Delay: %d\n", sprFrame.Delay );
			ConsolePrintf( "    Components\n" );

			ConsolePrintf( "     Name: %s/%s\n",
				sprFrame.SprAnimComponent->SprComponent->Name.c_str(),
				sprFrame.SprAnimComponent->SprComponentClipName.c_str() );
			ConsolePrintf( "     Offset: %d %d %d\n",
				sprFrame.SprAnimComponent->FrameOffsetX,
				sprFrame.SprAnimComponent->FrameOffsetY,
				sprFrame.SprAnimComponent->FrameZOrder );
		}
	}
}
//---------------------------------------