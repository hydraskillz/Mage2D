/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class Tile
	{
	public:
		Tile();

		int TileId;			// Tile ID used in tileset
		int TileIndex;		// Index into Map tile array
		int TileSetIndex;	// Index into tileset array
		int TilePositionX;	// Position in Map space
		int TilePositionY;
		int TileCollision;	// How Entities react to collision with this tile

		enum TileCollisionFlag
		{
			TC_NONE,
			TC_SOLID
		};
	};

}