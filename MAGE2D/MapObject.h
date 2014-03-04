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

	class MapObject
	{
	public:
		enum ObjectType
		{
			Obj_NONE,
			Obj_SPAWN,
			Obj_TRIGGER,
			Obj_CAMERA_BOUNDS,
			Obj_PATH
		};

		enum ObjectShape
		{
			Obj_RECT,
			Obj_ELLIPSE,
			Obj_POLYGON,
			Obj_POLYLINE
		};

		static int DecodeTypeString( const std::string& str );

		MapObject( const std::string& name, int type,
			const RectI& rect, bool visible=true );
		~MapObject();

		const std::string Name;
		const ObjectType Type;
		RectI BoundingRect;

		ObjectShape Shape;
		Vec2i Origin;
		std::vector< Vec2i > Verticies;

		bool Visible;

		Dictionary Properties;
		
		void OnDraw( SDL_Surface* screen );

		// Check properties and set members
		void EvalProperties();
		// Do stuff on events
		void OnTriggered( Dictionary& params );

		static void DestroyAllObjects();
	private:
		std::vector< std::string > mEntityFlags;
		std::string mTeleportTargetName;
		std::string mFireEventName;
		static std::map< std::string, MapObject* > msMapObjects;

		MapObject& operator= ( const MapObject& other );
	};

}