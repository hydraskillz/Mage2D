/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   
 */
 
#pragma once

struct SDL_Surface;

namespace mage
{

	class Map
	{
	public:
		Map();
		~Map();

		bool Load( const char* filename );
		void DestroyMap();
		void OnDraw( SDL_Surface* screen, const Camera& camera );
		void OnUpdate( float dt );

		inline int GetTileWidth() const;
		inline int GetTileHeight() const;
		RectI GetWorldBounds() const;

		Tile& GetTile( int x, int y, unsigned int layerIndex=0U ) const;

		void SpawnGameObjectEvent( Dictionary& params );
		const std::vector< GameObject* >& GetAllObjects() const { return mObjects; }
		GameObject* GetFirstObjectOfName( const std::string& name ) const;
		void SetCameraTarget( GameObject* target );
		void SetCameraBounds( const RectI& bounds );
		void SetCamera( Camera* camera ) { mCamera = camera; }

		void ToggleObjectDebug() { mObjectDebugDraw = !mObjectDebugDraw; }

		Vec2f Gravity;

		static Tile INVALID_TILE;
		static int CollisionLayerIndex;

	private:
		static const float PHYSICS_TIMESTEP;

		// Type of map layer
		enum LayerType
		{
			LT_NONE,
			LT_TILE,
			LT_OBJ,
			LT_IMG
		};

		// Map layer base
		struct MapLayer
		{
			virtual ~MapLayer();

			LayerType Type;
			bool Visible;
			float Opacity;
		};

		// Map layer types
		struct TileLayer : public MapLayer
		{
			std::vector< Tile > Tiles;
		};

		struct ObjLayer : public MapLayer
		{
			virtual ~ObjLayer();
			std::vector< GameObject* > Objs;
		};

		struct ImgLayer : public MapLayer
		{
			SDL_Surface* ImageSurface;
			Vec2i ScrollOffset;
			Vec2i ScrollSpeed;
		};

		// Tileset
		struct TileSet
		{
			int TileSetIndex;
			unsigned int FirstGid;
			SDL_Surface* TilesetSurface;
			std::map< int, Dictionary > TileProperties;
		};

		// itr is <tileset>
		void LoadTileset( const XmlReader::XmlReaderIterator& itr );
		// itr is <layer>
		void LoadTileLayer( const XmlReader::XmlReaderIterator& itr );
		// itr is <objectgroup>
		void LoadObjectGroup( const XmlReader::XmlReaderIterator& itr );
		// itr is <imagelayer>
		void LoadImageLayer( const XmlReader::XmlReaderIterator& itr );
		// itr is <image>
		SDL_Surface* LoadImage( const XmlReader::XmlReaderIterator& itr );
		// itr is <properties>
		void LoadProperties( Dictionary& props, const XmlReader::XmlReaderIterator& itr );
		void LoadProperties( GameObject* gameObject, const XmlReader::XmlReaderIterator& itr );
		// Load general layer info from a <layer> tag
		void LoadLayerBaseInfo( MapLayer* layer, const XmlReader::XmlReaderIterator& itr );

		std::vector< MapLayer* > mLayers;
		std::vector< TileSet* > mTileSets;
		std::vector< GameObject* > mObjects;

		int mMapWidth;
		int mMapHeight;
		int mTileWidth;
		int mTileHeight;

		std::string mMapDirectory;
		std::string mCurrentPath;

		Camera* mCamera;
		GameObject* mCameraTarget;

		bool mObjectDebugDraw;
	};

#include "Map.hpp"

}