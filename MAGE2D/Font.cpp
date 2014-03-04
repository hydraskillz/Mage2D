#include "MageLib.h"
#include "Surface.h"

#include <SDL_ttf.h>

using namespace mage;

Font* Font::DefaultFont = NULL;

//---------------------------------------
Font::Font( const char* filename, int fontSize )
	: mFont( NULL )
{
	LoadFont( filename, fontSize );
}
//---------------------------------------
Font::~Font()
{
	if ( mFont )
	{
		TTF_CloseFont( mFont );
	}
}
//---------------------------------------
bool Font::LoadFont( const char* filename, int fontSize )
{
	SDL_RWops* rw;
	char* buffer=0;
	unsigned int size;
	if ( OpenDataFile( filename, buffer, size ) == FSE_NO_ERROR )
	{
		rw = SDL_RWFromMem( buffer, size );
		mFont = TTF_OpenFontRW( rw, 1, fontSize );
		//mFont = TTF_OpenFont( filename, fontSize );
	}
	
	if ( !mFont )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to load font '%s'\n", filename );
		ConsolePrintf( CONSOLE_ERROR, " %s\n", TTF_GetError() );
		return false;
	}

	return true;
}
//---------------------------------------
void Font::RenderTextDynamic( SDL_Surface* dst, int x, int y, const char* text,
	const Color& color, int fontQuality, const Color& bgColor )
{
	// Make sure arguments are valid
	if ( mFont && text && dst )
	{
		SDL_Surface* tmpSurf = RenderTextStatic( text, color, fontQuality, bgColor );
		
		if ( tmpSurf )
		{
			// Render the text to the dst surface
			Surface::DrawSurface( dst, tmpSurf, x, y );

			// Free the surface
			SDL_FreeSurface( tmpSurf );
		}
	}
}
//---------------------------------------
SDL_Surface* Font::RenderTextStatic( const char* text, const Color& color,
	int fontQuality, const Color& bgColor )
{
	SDL_Surface* resultSurf = NULL;

	// Make sure arguments are valid
	if ( mFont && text )
	{
		SDL_Color fg = { color.r, color.g, color.b };
		SDL_Color bg = { bgColor.r, bgColor.g, bgColor.b };

		switch ( fontQuality )
		{
			case Font_NICE:
			{
				resultSurf = TTF_RenderText_Shaded( mFont, text, fg, bg );
				break;
			}
			case Font_ULTRA:
			{
				resultSurf = TTF_RenderText_Blended( mFont, text, fg );
				break;
			}
			// Default case is FAST
			case Font_FAST:
			default:
			{
				resultSurf = TTF_RenderText_Solid( mFont, text, fg );
				break;
			}
		}
	}

	return resultSurf;
}
//---------------------------------------
int Font::GetRecomendedLineSpacing() const
{
	return TTF_FontLineSkip( mFont );
}
//---------------------------------------
int Font::GetFontHeight() const
{
	return TTF_FontHeight( mFont );
}
//---------------------------------------
int Font::GetTextWidth( const char* text ) const
{
	int w, h;
	GetTextSize( text, w, h );
	return w;
}
//---------------------------------------
void Font::GetTextSize( const char* text, int& width, int& height ) const
{
	TTF_SizeText( mFont, text, &width, &height );
}
//---------------------------------------