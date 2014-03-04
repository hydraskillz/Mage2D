#include "MageLib.h"

using namespace mage;

//---------------------------------------
GameNodeEditor::GameNodeEditor()
	: mGameObjectDef( NULL )
{}
//---------------------------------------
GameNodeEditor::~GameNodeEditor()
{}
//---------------------------------------
bool GameNodeEditor::Load( const char* filename )
{
	Delete0( mGameObjectDef );
	mGameObjectDef = new GameObjectDefinition( filename );
	return mGameObjectDef != NULL;
}
//---------------------------------------
void GameNodeEditor::SetActiveDefinition( GameObjectDefinition* def )
{
	mGameObjectDef = def;
	mOffset[0] = 0;
	mOffset[1] = 0;
	mOffset[2] = 0;
	mCellHeight = Font::DefaultFont->GetFontHeight() * 2.0f;
}
//---------------------------------------
void GameNodeEditor::OnDraw( SDL_Surface* screen ) const
{
	if ( mGameObjectDef )
	{
		Vec2i pos( 275, mOffset[1] );
		int h = mCellHeight;//Font::DefaultFont->GetFontHeight() * 2.0f;
		int w = 180;
		int deltaY = h * 1.25f;

		Vec2i trigPos( 0, mOffset[0] );

		Vec2i actPos( pos.x + w + 95, mOffset[2] );

		for ( auto itr = mGameObjectDef->mControllerDefinitions.begin(); itr != mGameObjectDef->mControllerDefinitions.end(); ++itr )
		{
			GameObjectDefinition::ControllerDefinition& cont = **itr;

			Surface::DrawRect( screen, pos.x, pos.y, w, h, Color::DARK_BLUE.bgra );
			Surface::DrawRect( screen, pos.x, pos.y, w, h, Color::BLUE.bgra, false );
			Surface::DrawText( screen, pos.x, pos.y, Font::DefaultFont, Color::WHITE.bgra,
				cont.mName.c_str() );
			Surface::DrawTextFormat( screen, pos.x + w - 32, pos.y, Font::DefaultFont, Color::WHITE.bgra,
				"%u", cont.mState );
			Surface::DrawText( screen, pos.x + w - 32, pos.y + Font::DefaultFont->GetFontHeight(), Font::DefaultFont, Color::WHITE.bgra,
				GameController::GameControllerTypeToCString( cont.mType ) );

			for ( auto trigItr = cont.mTriggerNames.begin(); trigItr != cont.mTriggerNames.end(); ++trigItr )
			{
				Surface::DrawLine( screen, trigPos.x + w, trigPos.y + h * 0.5f,
					pos.x, pos.y + h * 0.5f, Color::GREEN.bgra );

				Surface::DrawRect( screen, trigPos.x, trigPos.y, w, h, Color::DARK_GREEN.bgra );
				Surface::DrawRect( screen, trigPos.x, trigPos.y, w, h, Color::GREEN.bgra, false );
				Surface::DrawText( screen, trigPos.x, trigPos.y, Font::DefaultFont, Color::WHITE.bgra,
					trigItr->c_str() );

				trigPos.y += deltaY;
			}

			for ( auto actItr = cont.mActionNames.begin(); actItr != cont.mActionNames.end(); ++actItr )
			{
				Surface::DrawLine( screen, actPos.x, actPos.y + h * 0.5f,
					pos.x + w, pos.y + h * 0.5f, Color::PINK.bgra );

				Surface::DrawRect( screen, actPos.x, actPos.y, w, h, Color::PURPLE.bgra );
				Surface::DrawRect( screen, actPos.x, actPos.y, w, h, Color::PINK.bgra, false );
				Surface::DrawText( screen, actPos.x, actPos.y, Font::DefaultFont, Color::WHITE.bgra,
					actItr->c_str() );

				actPos.y += deltaY;
			}

			pos.y += deltaY;
		}
	}
}
//---------------------------------------
void GameNodeEditor::Scroll( int dir, int col )
{
	mOffset[col] += dir * mCellHeight;
}
//---------------------------------------