/*
 * Author      : Matthew Johnson
 * Date        : 9/Jul/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameNodeEditor
	{
	public:
		GameNodeEditor();
		~GameNodeEditor();

		bool Load( const char* filename );
		void SetActiveDefinition( GameObjectDefinition* def );
		void OnDraw( SDL_Surface* screen ) const;
		void Scroll( int dir, int col );

	private:
		GameObjectDefinition* mGameObjectDef;
		int mOffset[3];
		int mCellHeight;
	};

}