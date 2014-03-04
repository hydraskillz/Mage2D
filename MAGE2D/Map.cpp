#include "MageLib.h"
#include "base64.h"

#include <SDL.h>

using namespace mage;

const unsigned int TMX_FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
const unsigned int TMX_FLIPPED_VERTICALLY_FLAG   = 0x40000000;
const unsigned int TMX_FLIPPED_DIAGONALLY_FLAG   = 0x20000000;

const float Map::PHYSICS_TIMESTEP = 1.0f / 60.0f;
Tile Map::INVALID_TILE;
int Map::CollisionLayerIndex = 0;

//---------------------------------------
Map::Map()
	: Gravity( 0.0f, 9.8f )
	, mCamera( NULL )
	, mCameraTarget( NULL )
	, mObjectDebugDraw( false )
{
	EventManager::RegisterObjectForEvent( "SpawnGameObject", *this, &Map::SpawnGameObjectEvent );
}
//---------------------------------------
Map::~Map()
{
	DestroyMap();
	EventManager::UnregisterObjectForAllEvent( *this );
}
//---------------------------------------
bool Map::Load( const char* filename )
{
	XmlReader reader( filename );
	XmlReader::XmlReaderIterator xmlRoot = reader.ReadRoot();

	// Ensure file exists before destroying old map
	if ( !xmlRoot.IsValid() )
	{
		return false;
	}

	// Clear old map data
	DestroyMap();


	mMapDirectory = std::string( filename ).substr( 0, std::string( filename ).find_last_of( "/" ) + 1 );

	// Map/Tile size
	xmlRoot.ValidateXMLAttributes( "width,height,tilewidth,tileheight","orientation,version" );
	mMapWidth = xmlRoot.GetAttributeAsInt( "width" );
	mMapHeight = xmlRoot.GetAttributeAsInt( "height" );
	mTileWidth = xmlRoot.GetAttributeAsInt( "tilewidth" );
	mTileHeight = xmlRoot.GetAttributeAsInt( "tileheight" );

	// Custom properties
	if ( xmlRoot.NextChild( "properties" ).IsValid() )
	{
		for ( XmlReader::XmlReaderIterator propItr = xmlRoot.NextChild( "properties" ).NextChild( "property" );
			propItr.IsValid(); propItr = propItr.NextSibling( "property" ) )
		{
			std::string k = propItr.GetAttributeAsString( "name" );
			if ( k == "Gravity" )
				Gravity = propItr.GetAttributeAsVec2f( "value", Vec2f( 0, 9.8f ) );
		}
	}
	

	// Load Map data
	for ( XmlReader::XmlReaderIterator itr = xmlRoot.NextChild();
		itr.IsValid(); itr = itr.NextSibling() )
	{
		if ( itr.ElementNameEquals( "properties" ) )
		{
		}
		else if ( itr.ElementNameEquals( "tileset" ) )
		{
			LoadTileset( itr );
		}
		else if ( itr.ElementNameEquals( "layer" ) )
		{
			LoadTileLayer( itr );
		}
		else if ( itr.ElementNameEquals( "objectgroup" ) )
		{
			LoadObjectGroup( itr );
		}
		else if ( itr.ElementNameEquals( "imagelayer" ) )
		{
			LoadImageLayer( itr );
		}
		else
		{
			ConsolePrintf( CONSOLE_WARNING, "Found unknown tmx tag: %s\n", itr.ElementName() );
		}
	}

	// Set the camera bounds to the entire map
	Dictionary params;
	params.Set( "Bounds", GetWorldBounds() );
	EventManager::FireEvent( "SetCameraBounds", params );

	// Signal the map is done loading
	EventManager::FireEvent( "OnMapLoaded" );

	return true;
}
//---------------------------------------
void Map::DestroyMap()
{
	// Clear tilesets
	for ( auto itr = mTileSets.begin(); itr != mTileSets.end(); ++itr )
	{
		SDL_FreeSurface( ( *itr )->TilesetSurface );
		Delete0( *itr );
	}

	mTileSets.clear();

	// Clear layers
	DestroyVector( mLayers );

	// Destroy all entities
	Entity::DestroyAllEntities();

	// Destroy all GameObjects
	DestroyVector( mObjects );
	mCameraTarget = NULL;

	// Destroy Sprites
	SpriteManager::Manager.DestroyAllSprites();
}
//---------------------------------------
void Map::LoadTileset( const XmlReader::XmlReaderIterator& itr )
{
	TileSet* tileset = new TileSet;

	tileset->FirstGid = itr.GetAttributeAsInt( "firstgid" );
	const char* source = itr.GetAttributeAsCString( "source", 0 );

	// Inline tileset
	if ( !source )
	{
		XmlReader::XmlReaderIterator imageTag = itr.NextChild( "image" );
		mCurrentPath = mMapDirectory;
		tileset->TilesetSurface = LoadImage( imageTag );
	}
	// External tileset
	else
	{
		std::string tilesetPath = mMapDirectory;
		tilesetPath += std::string( source );
		std::string tilesetDirectory = tilesetPath.substr( 0, tilesetPath.find_last_of( "/" ) + 1 );
		tilesetPath = StringUtil::ExtractFilenameFromPath( tilesetPath );
		XmlReader tilesetReader( ( std::string( "tilesets/" ) + tilesetPath ).c_str() );
		XmlReader::XmlReaderIterator tilesetRoot = tilesetReader.ReadRoot();

		XmlReader::XmlReaderIterator imageTag = tilesetRoot.NextChild( "image" );
		mCurrentPath = tilesetDirectory;
		tileset->TilesetSurface = LoadImage( imageTag );

		for ( XmlReader::XmlReaderIterator tileItr = tilesetRoot.NextChild( "tile" );
			  tileItr.IsValid(); tileItr = tileItr.NextSibling( "tile" ) )
		{
			tileItr.ValidateXMLAttributes( "id","" );
			tileItr.ValidateXMLChildElemnts( "properties","" );

			int id = tileItr.GetAttributeAsInt( "id" );

			LoadProperties( tileset->TileProperties[ id ], tileItr.NextChild( "properties" ) );
		}
	}

	tileset->TileSetIndex = (int) mTileSets.size();
	mTileSets.push_back( tileset );
}
//---------------------------------------
void Map::LoadTileLayer( const XmlReader::XmlReaderIterator& itr )
{
	TileLayer* layer = New0 TileLayer;
	layer->Type = LT_TILE;

	LoadLayerBaseInfo( layer, itr );

	itr.ValidateXMLChildElemnts( "data","tile,properties" );
	XmlReader::XmlReaderIterator data = itr.NextChild( "data" );

	const char* encoding = data.GetAttributeAsCString( "encoding", 0 );
	const char* compression = data.GetAttributeAsCString( "compression", 0 );

	// Tile data is encoded
	if ( encoding )
	{
		// Base64 encoded
		if ( !strcmp( encoding, "base64" ) )
		{
			// Get encoded data
			std::string rawData = data.GetPCDataAsString();

			// Decode data
			std::vector< int > tiledata = base64_decode( rawData );

			// Tile data is compressed
			if ( compression )
			{
				if ( !strcmp( compression, "gzip" ) )
				{
					ConsolePrintf( CONSOLE_WARNING, "Warning: gzip compression not supported yet... no data loaded\n" );
					return;
				}
				else if ( !strcmp( compression, "zlib" ) )
				{
					ConsolePrintf( CONSOLE_WARNING, "Warning: zlib compression not supported yet... no data loaded\n" );
					return;
				}
				else
				{
					ConsolePrintf( CONSOLE_WARNING, "Warning: Unknown compression: %s\n", compression );
					return;
				}
			}
			
			int k = 0, size = (int) tiledata.size();
			for ( int j = 0; j < size; j += 4 )
			{
				Tile tile;

				// Get global tile id
				unsigned int gid = tiledata[ j     ]       |
								   tiledata[ j + 1 ] << 8  |
								   tiledata[ j + 2 ] << 16 |
								   tiledata[ j + 3 ] << 24;

				// Read tile flags (unused)
				//bool flippedHorizontally = ( gid & TMX_FLIPPED_HORIZONTALLY_FLAG ) == 0 ? false : true;
				//bool flippedVertically   = ( gid & TMX_FLIPPED_VERTICALLY_FLAG   ) == 0 ? false : true;
				//bool flippedDiagonally   = ( gid & TMX_FLIPPED_DIAGONALLY_FLAG   ) == 0 ? false : true;

				// Clear flags
				gid &= ~( TMX_FLIPPED_HORIZONTALLY_FLAG |
					      TMX_FLIPPED_VERTICALLY_FLAG   |
					      TMX_FLIPPED_DIAGONALLY_FLAG );

				// Figure out which tileset this tile belongs to
				if ( gid > 0 )
				{
					int tilesetIndex = 0;
					for ( int i = (int) mTileSets.size() - 1; i >= 0; --i )
					{
						TileSet* tileset = mTileSets[i];
					
						if ( tileset->FirstGid <= gid )
						{
							tilesetIndex = tileset->TileSetIndex;
							// Fix gid to be local to this tileset
							gid = gid - tileset->FirstGid;
							break;
						}
					}

					const int numXTiles = mTileSets[ tilesetIndex ]->TilesetSurface ? mTileSets[ tilesetIndex ]->TilesetSurface->w / mTileWidth : 1;
				
					tile.TileId = gid;
					tile.TileIndex = k++;
					tile.TileSetIndex = tilesetIndex;
					tile.TilePositionX = ( gid % numXTiles ) * mTileWidth;
					tile.TilePositionY = ( gid / numXTiles ) * mTileHeight;

					auto tileProps = mTileSets[ tilesetIndex ]->TileProperties.find( gid );
					if ( tileProps != mTileSets[ tilesetIndex ]->TileProperties.end() )
					{
						// Check for collision properties
						std::string collisionType;
						if ( tileProps->second.Get( "Collision", collisionType ) == Dictionary::DErr_SUCCESS )
						{
							if ( collisionType == "SOLID" )
							{
								tile.TileCollision = Tile::TC_SOLID;
							}
						}
					}
				}

				layer->Tiles.push_back( tile );
			}
		}
		// CSV encoded
		else if ( !strcmp( encoding, "csv" ) )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: CSV encoding not supported yet... no data loaded\n" );
		}
		else
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: Unknown encoding: %s\n", encoding );
		}
	}
	// Raw XML tile data
	else
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: Raw tile data not supported yet... no data loaded\n" );
	}

	XmlReader::XmlReaderIterator propItr = itr.NextChild( "properties" );
	if ( propItr.IsValid() )
	{
		Dictionary layerProperties;
		LoadProperties( layerProperties, propItr );

		// The 'Collision' property specifies a layer is to be used for collision
		std::string _dummy_;
		if ( layerProperties.Get( "Collision", _dummy_ ) == Dictionary::DErr_SUCCESS )
		{
			CollisionLayerIndex = (int) mLayers.size();
		}
	}

	mLayers.push_back( layer );
}
//---------------------------------------
void Map::LoadObjectGroup( const XmlReader::XmlReaderIterator& itr )
{
	ObjLayer* layer = new ObjLayer;
	layer->Type = LT_OBJ;

	LoadLayerBaseInfo( layer, itr );

	for ( XmlReader::XmlReaderIterator objItr = itr.NextChild( "object" );
		objItr.IsValid(); objItr = objItr.NextSibling( "object" ) )
	{
		objItr.ValidateXMLAttributes( "x,y","name,width,height,type,visible" );

		std::string name = objItr.GetAttributeAsString( "name", "" );
		std::string type = objItr.GetAttributeAsString( "type", "" );
		int x = objItr.GetAttributeAsInt( "x" );
		int y = objItr.GetAttributeAsInt( "y" );
		int w = objItr.GetAttributeAsInt( "width", 0 );
		int h = objItr.GetAttributeAsInt( "height", 0 );
		bool visible = objItr.GetAttributeAsInt( "visible", 1 ) == 0 ? false : true;

		GameObject* gameObject;
		GameObjectDefinition* def = GameObjectDefinition::GetDefinitionByName( type );
		if ( def )
		{
			gameObject = def->Create();

			// Set attributes
			gameObject->Visible = layer->Visible;//visible;
			gameObject->Position.x = (float) x;
			gameObject->Position.y = (float) y;
			gameObject->BoundingRect.Set( x, y, x + w, y + h );
			gameObject->GameMap = this;
			gameObject->ObjectId = mObjects.size();

			// Load editor values for properties
			XmlReader::XmlReaderIterator propItr = objItr.NextChild( "properties" );
			if ( propItr.IsValid() )
			{
				LoadProperties( gameObject, propItr );
			}

			// Call OnCreate for all logic nodes on the object
			gameObject->FinalizeNodes();
		}
		else
		{
			ConsolePrintf( CONSOLE_WARNING, "Unknown object type: '%s'\n", type.c_str() );
			continue;
		}

		layer->Objs.push_back( gameObject );
		mObjects.push_back( gameObject );

		/*
		MapObject* mapObject = new MapObject(
			name, type, RectI( x, y, x + w, y + h ), visible );

		// Read properties
		XmlReader::XmlReaderIterator propItr = objItr.NextChild( "properties" );
		if ( propItr.IsValid() )
		{
			LoadProperties( mapObject->Properties, propItr );
			mapObject->EvalProperties();
		}

		// Check for polyline
		XmlReader::XmlReaderIterator polylineItr = objItr.NextChild( "polyline" );
		if ( polylineItr.IsValid() )
		{
			polylineItr.ValidateXMLAttributes( "points","" );

			std::string pointCSV = polylineItr.GetAttributeAsString( "points" );
			std::vector< std::string > csvTokens;
			std::vector< std::string > csvTokenElements;
			StringUtil::Tokenize( pointCSV, csvTokens, " " );
			int xmin=0, xmax=0, ymin=0, ymax=0;
			for ( auto csvItr = csvTokens.begin(); csvItr != csvTokens.end(); ++csvItr )
			{
				StringUtil::Tokenize( *csvItr, csvTokenElements, "," );

				if ( csvTokenElements.size() == 2U )
				{
					int vx, vy;

					if ( StringUtil::StringToType( csvTokenElements[0], &vx ) &&
						 StringUtil::StringToType( csvTokenElements[1], &vy ) )
					{
						mapObject->Verticies.push_back( Vec2i( vx, vy ) );

						if ( vx > xmax ) xmax = vx;
						if ( vx < xmin ) xmin = vx;
						if ( vy > ymax ) ymax = vy;
						if ( vy < ymin ) ymin = vy;
					}
				}

				csvTokenElements.clear();
			}

			mapObject->Shape = MapObject::Obj_POLYLINE;
			mapObject->BoundingRect.Set( x+xmin, y+ymin, x+xmax, y+ymax );
		}
		// Rect
		else
		{
			mapObject->Shape = MapObject::Obj_RECT;
			mapObject->Verticies.push_back( Vec2i( x, y ) );
			mapObject->Verticies.push_back( Vec2i( x+w, y ) );
			mapObject->Verticies.push_back( Vec2i( x+w, y+h ) );
			mapObject->Verticies.push_back( Vec2i( x, y+h ) );
		}

		layer->Objs.push_back( mapObject );
		*/
	}
	mLayers.push_back( layer );
}
//---------------------------------------
void Map::LoadImageLayer( const XmlReader::XmlReaderIterator& itr )
{
	ImgLayer* layer = new ImgLayer();
	layer->Type = LT_IMG;

	LoadLayerBaseInfo( layer, itr );

	XmlReader::XmlReaderIterator imageItr = itr.NextChild( "image" );

	if ( imageItr.IsValid() )
	{
		layer->ImageSurface = LoadImage( imageItr );

		// Custom properties
		if ( itr.NextChild( "properties" ).IsValid() )
		{
			for ( XmlReader::XmlReaderIterator propItr = itr.NextChild( "properties" ).NextChild( "property" );
				propItr.IsValid(); propItr = propItr.NextSibling( "property" ) )
			{
				std::string k = propItr.GetAttributeAsString( "name" );
				if ( k == "ScrollSpeed" )
					layer->ScrollSpeed = (Vec2i) propItr.GetAttributeAsVec2f( "value", Vec2f::ZERO );
			}
		}
	}
	else
	{
		layer->ImageSurface = NULL;
		ConsolePrintf( CONSOLE_WARNING, "Warning: Empty image layer - is this intentional?\n" );
	}

	mLayers.push_back( layer );
}
//---------------------------------------
SDL_Surface* Map::LoadImage( const XmlReader::XmlReaderIterator& itr )
{
	itr.ValidateXMLAttributes( "source","width,height,trans" );

	// Build image path
	const char* imgSource = itr.GetAttributeAsCString( "source" );
	std::string sourcePath = mCurrentPath;
	sourcePath += std::string( imgSource );

	SDL_Surface* img;

	char* buffer=0;
	unsigned int size;
	if ( OpenDataFile( ( std::string( "tilesets/" ) + std::string( imgSource ) ).c_str(), buffer, size ) != FSE_NO_ERROR )
	{
		ConsolePrintf( CONSOLE_WARNING, "Failed to load texture '%s'\n", imgSource );
		return NULL;
	}

	// Check for colorkey flag
	std::string colorkeyStr = itr.GetAttributeAsString( "trans", "NONE" );
	if ( colorkeyStr != "NONE" )
	{
		// Load image without alpha
		img = Surface::LoadSurfaceMem( buffer, size, 1, false );
		//img = Surface::LoadSurface( sourcePath.c_str(), false );

		if ( !img )
		{
			ConsolePrintf( CONSOLE_WARNING, "Failed to load texture '%s'\n", imgSource );
			return NULL;
		}

		// Get colorkey from string and apply it to the image
		unsigned int colorkey;
		if ( StringUtil::HexStringToUnsigned( colorkeyStr, colorkey ) )
		{
			Surface::SetColorKey( img, colorkey );
		}
	}
	else
	{
		// Load image with alpha
		//img = Surface::LoadSurface( sourcePath.c_str() );
		img = Surface::LoadSurfaceMem( buffer, size, 1, true );
	}

	return img;
}
//---------------------------------------
void Map::LoadProperties( Dictionary& props, const XmlReader::XmlReaderIterator& itr )
{
	for ( XmlReader::XmlReaderIterator propItr = itr.NextChild( "property" );
		propItr.IsValid(); propItr = propItr.NextSibling( "property" ) )
	{
		std::string k = propItr.GetAttributeAsString( "name" );
		std::string v = propItr.GetAttributeAsString( "value" );
		props.Set( k, v );
	}
}
//---------------------------------------
void Map::LoadProperties( GameObject* gameObject, const XmlReader::XmlReaderIterator& itr )
{
	for ( XmlReader::XmlReaderIterator propItr = itr.NextChild( "property" );
		propItr.IsValid(); propItr = propItr.NextSibling( "property" ) )
	{
		std::string k = propItr.GetAttributeAsString( "name" );
		std::string v = propItr.GetAttributeAsString( "value" );
		gameObject->SetProperty( k, v );
	}
}
//---------------------------------------
void Map::LoadLayerBaseInfo( MapLayer* layer, const XmlReader::XmlReaderIterator& itr )
{
	// Validate layer tag
	itr.ValidateXMLAttributes( "name","x,y,width,height,opacity,visible" );

	// Read in attributes
	layer->Visible = itr.GetAttributeAsInt( "visible", 1 ) == 0 ? false : true;
	layer->Opacity = itr.GetAttributeAsFloat( "opacity", 1.0f );
}
//---------------------------------------
void Map::OnDraw( SDL_Surface* screen, const Camera& camera )
{
	// Get visible range of tiles
	int cameraX = (int) camera.GetX();// - ( ((int) camera.GetViewWidth()) % mTileWidth );
	int cameraY = (int) camera.GetY();// - ( ((int) camera.GetViewHeight()) % mTileHeight );
	int startX = (int) ( camera.GetX() / mTileWidth );
	int startY = (int) ( camera.GetY() / mTileHeight );
	int endX = startX + camera.GetViewWidth() / mTileWidth;// - 1;
	int endY = startY + camera.GetViewHeight() / mTileHeight;// - 1;

	Mathi::ClampToRange( startX, 0, mMapWidth  - 1 );
	Mathi::ClampToRange( startY, 0, mMapHeight - 1 );
	Mathi::ClampToRange( endX,   0, mMapWidth  - 1 );
	Mathi::ClampToRange( endY,   0, mMapHeight - 1 );

	for ( auto layerItr = mLayers.begin(); layerItr != mLayers.end(); ++layerItr )
	{
		MapLayer* layer = *layerItr;

		// Skip invisible layers
		if ( !layer->Visible || layer->Opacity == 0.0f ) continue;


		// Tile layer
		if ( layer->Type == LT_TILE )
		{
			TileLayer* tileLayer = (TileLayer*) layer;

			// Loop and draw all visible tiles in the viewport
			for ( int y = startY; y <= endY; ++y )
			{
				for ( int x = startX; x <= endX; ++x )
				{
					int index = x + y * mMapWidth;

					// Draw tile if id is valid
					if ( tileLayer->Tiles[ index ].TileId >= 0 )
					{
						const Tile& tile = tileLayer->Tiles[ index ];

						Surface::DrawSurface( screen, mTileSets[ tile.TileSetIndex ]->TilesetSurface,
							x * mTileWidth - cameraX,
							y * mTileHeight - cameraY,
							tile.TilePositionX, tile.TilePositionY, mTileWidth, mTileHeight );
					}
				}
			}
		}

		// Object layer
		else if ( layer->Type == LT_OBJ )
		{
			continue;
			/*
			ObjLayer* objLayer = (ObjLayer*) layer;

			for ( auto itr = objLayer->Objs.begin(); itr != objLayer->Objs.end(); ++itr )
			{
				GameObject* obj = *itr;

				obj->OnDraw( screen, camera );

				/*
				// Check if we can see the object before we draw it
				if ( camera.RectInViewport( obj->BoundingRect ) )
				{
					int originX = obj->Position.x - cameraX;
					int originY = obj->Position.y - cameraY;

					// Outline
					Surface::DrawRect(
						screen,
						obj->BoundingRect.Left - cameraX,
						obj->BoundingRect.Top - cameraY,
						obj->BoundingRect.Width(),
						obj->BoundingRect.Height(),
						0xffffffff, false );

					//if ( obj->Shape == MapObject::Obj_RECT )
					//{
						// Transparent fill
						Surface::DrawRect(
							screen,
							obj->BoundingRect.Left - cameraX,
							obj->BoundingRect.Top - cameraY,
							obj->BoundingRect.Width(),
							obj->BoundingRect.Height(),
							0xaaaaaaaa );
					//}
					//else if ( obj->Shape == MapObject::Obj_POLYLINE )
					//{
					//	// Path
					//	int lastX = 0, lastY = 0;
					//	for ( auto vertItr = obj->Verticies.begin();
					//		vertItr != obj->Verticies.end(); ++vertItr )
					//	{
					//		Surface::DrawLine( screen, lastX + originX, lastY + originY,
					//			vertItr->x + originX, vertItr->y + originY,
					//			0xff00ffff );
					//		lastX = vertItr->x;
					//		lastY = vertItr->y;
					//	}
					//}

					// Origin
					Surface::DrawRect( screen, originX - 1, originY - 1, 2, 2,
						0xffff0000, true );

					// Center
					Surface::DrawRect( screen,
						obj->BoundingRect.Left + obj->BoundingRect.CenterX() - cameraX - 1,
						obj->BoundingRect.Top + obj->BoundingRect.CenterY() - cameraY - 1,
						2, 2,
						0xff0000ff, true );
				}
			}*/
		}

		// Image layer
		else if ( layer->Type == LT_IMG )
		{
			ImgLayer* imgLayer = (ImgLayer*) layer;

			if ( imgLayer->ImageSurface )
			{
				Surface::DrawSurface( screen, imgLayer->ImageSurface,
					imgLayer->ScrollOffset.x, imgLayer->ScrollOffset.y );

				// @TODO some of this should go in update
				imgLayer->ScrollOffset += imgLayer->ScrollSpeed;

				const int width  = imgLayer->ImageSurface->w;
				const int height = imgLayer->ImageSurface->h;

				// Scroll left
				if ( imgLayer->ScrollSpeed.x < 0 )
				{
					if ( imgLayer->ScrollOffset.x + width < camera.GetViewWidth() )
					{
						Surface::DrawSurface( screen, imgLayer->ImageSurface,
							imgLayer->ScrollOffset.x + width, imgLayer->ScrollOffset.y );
					}

					if ( imgLayer->ScrollOffset.x + width < 0 )
					{
						imgLayer->ScrollOffset.x = 0;
					}
				}
				// Scroll right
				else if ( imgLayer->ScrollSpeed.x > 0 )
				{
					if ( imgLayer->ScrollOffset.x > 0 )
					{
						Surface::DrawSurface( screen, imgLayer->ImageSurface,
							imgLayer->ScrollOffset.x - width, imgLayer->ScrollOffset.y );
					}

					if ( imgLayer->ScrollOffset.x > camera.GetViewWidth() )
					{
						imgLayer->ScrollOffset.x = -width + camera.GetViewWidth();
					}
				}

				// Scroll up
				if ( imgLayer->ScrollSpeed.y < 0 )
				{
					if ( imgLayer->ScrollOffset.y + height < camera.GetViewHeight() )
					{
						Surface::DrawSurface( screen, imgLayer->ImageSurface,
							imgLayer->ScrollOffset.x, imgLayer->ScrollOffset.y + height );
					}

					if ( imgLayer->ScrollOffset.y + height < 0 )
					{
						imgLayer->ScrollOffset.y = 0;
					}
				}

				// Scroll down
				if ( imgLayer->ScrollSpeed.y > 0 )
				{
					if ( imgLayer->ScrollOffset.y > 0 )
					{
						Surface::DrawSurface( screen, imgLayer->ImageSurface,
							imgLayer->ScrollOffset.x, imgLayer->ScrollOffset.y - height );
					}

					if ( imgLayer->ScrollOffset.y > camera.GetViewHeight() )
					{
						imgLayer->ScrollOffset.y = -height + camera.GetViewHeight();
					}
				}
			}
		}
	}

	for ( auto itr = mObjects.begin(); itr != mObjects.end(); ++itr )
	{
		(*itr)->OnDraw( screen, camera );
	}
}
//---------------------------------------
void Map::OnUpdate( float dt )
{

	if ( mCameraTarget )
	{
		mCamera->LookAt( mCameraTarget->Position );
	}

	uint32 numObjects = mObjects.size();

	BeginProfilingSection( GameLogic );
	for ( uint32 i = 0; i < numObjects; ++i )
	{
		GameObject& gameObject = *mObjects[i];

		gameObject.OnUpdate( dt );
	}
	EndProfilingSection( GameLogic );

	BeginProfilingSection( Physics );
	for ( uint32 i = 0; i < numObjects; ++i )
	{
		GameObject& gameObject = *mObjects[i];

		gameObject.ApplyForce( Gravity );
		gameObject.OnUpdatePhysics( PHYSICS_TIMESTEP );
	}
	EndProfilingSection( Physics );

	/*
	BeginProfilingSection( Physics );
	Entity::ForEachEntity( [&]( Entity* entity )
	{
		entity->ApplyImpulse( Gravity );
		entity->OnUpdatePhysics( PHYSICS_TIMESTEP );
		entity->OnUpdate( dt );
	});
	EndProfilingSection( Physics );
	*/

	for ( uint32 i = 0; i < numObjects; ++i )
	{
		GameObject& gameObject = *mObjects[i];
		gameObject.mCollidingObjects.clear();
	}

	// Process collisions
	for ( auto itr = GameObject::CollisionInfo::sObjectCollisions.begin();
		  itr != GameObject::CollisionInfo::sObjectCollisions.end(); ++itr )
	{
		GameObject* A = itr->A;
		GameObject* B = itr->B;

		if ( A->OnCollision( B ) )
		{
			B->OnCollision( A );
		}
	}

	GameObject::CollisionInfo::sObjectCollisions.clear();

	// Remove dead objects
	mObjects.erase(
		std::remove_if( mObjects.begin(), mObjects.end(), [&]( GameObject*& object ) -> bool
	{
		if ( !object->IsValid() )
		{
			delete object;
			object = 0;
			return true;
		}
		return false;
	}), mObjects.end() );

}
//---------------------------------------
Tile& Map::GetTile( int x, int y, unsigned int layerIndex ) const
{
	MapLayer* layer = mLayers[ layerIndex ];
	if ( layer->Type == LT_TILE )
	{
		TileLayer* tileLayer = (TileLayer*) layer;
		unsigned int index = (unsigned int) ( x + mMapWidth * y );
		if ( index < 0 || index >= tileLayer->Tiles.size() )
		{
			return INVALID_TILE;
		}
		return tileLayer->Tiles[ index ];
	}

	return INVALID_TILE;
}
//---------------------------------------
RectI Map::GetWorldBounds() const
{
	return RectI( 0, 0, mTileWidth * mMapWidth, mTileHeight * mMapHeight );
}
//---------------------------------------
void Map::SpawnGameObjectEvent( Dictionary& params )
{
	std::string name;
	Vec2f position;

	// Get name
	if ( params.Get( "ObjectName", name ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn GameObject: missing ObjectName\n" );
		return;
	}

	// Get position
	if ( params.Get( "Position", position ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn GameObject: missing Position\n" );
		return;
	}

	// Spawn entity
	GameObject* gameObject = NULL;

	GameObjectDefinition* def = GameObjectDefinition::GetDefinitionByName( name );
	if ( def )
	{
		gameObject = def->Create();
		gameObject->Position = position;
		gameObject->GameMap = this;
		gameObject->SetBoundsFromSprite();
		gameObject->ObjectId = mObjects.size();

		// Call OnCreate for all logic nodes on the object
		gameObject->FinalizeNodes();

		mObjects.push_back( gameObject );
	}
	else
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn GameObject: No GameObject found '%s'\n", name.c_str() );
	}
	params.Set( "GameObject", gameObject );
}
//---------------------------------------
GameObject* Map::GetFirstObjectOfName( const std::string& name ) const
{
	for ( auto itr = mObjects.begin(); itr != mObjects.end(); ++itr )
	{
		GameObject* gameObject = *itr;

		if ( gameObject->Name == name )
		{
			return gameObject;
		}
	}
	return NULL;
}
//---------------------------------------
void Map::SetCameraTarget( GameObject* target )
{
	mCameraTarget = target;
}
//---------------------------------------
void Map::SetCameraBounds( const RectI& bounds )
{
	mCamera->SetWorldBounds( bounds );
}
//---------------------------------------


//---------------------------------------
// Map Layers
//---------------------------------------
Map::MapLayer::~MapLayer()
{}
//---------------------------------------
Map::ObjLayer::~ObjLayer()
{
	//MapObject::DestroyAllObjects();
	//DestroyVector( Objs );
	Objs.clear();
}
//---------------------------------------