#include "MageLib.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>

using namespace mage;

//---------------------------------------
Surface::Surface()
{}
//---------------------------------------
SDL_Surface* Surface::LoadSurface( const char* filename, bool formatAlpha )
{
	SDL_Surface* tmpSurf = NULL;
	SDL_Surface* formatSurf = NULL;

	tmpSurf = IMG_Load( filename );
	if ( !tmpSurf )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to load image: %s\n", filename );
		return NULL;
	}

	if ( formatAlpha )
	{
		formatSurf = SDL_DisplayFormatAlpha( tmpSurf );
	}
	else
	{
		formatSurf = SDL_DisplayFormat( tmpSurf );
	}
	SDL_FreeSurface( tmpSurf );

	return formatSurf;
}
//---------------------------------------
SDL_Surface* Surface::LoadSurfaceMem( void* imgBuffer, int bufferSize, int freeBuffer, bool formatAlpha )
{
	SDL_Surface* tmpSurf = NULL;
	SDL_Surface* formatSurf = NULL;
	SDL_RWops* rw;
	
	rw = SDL_RWFromMem( imgBuffer, bufferSize );
	tmpSurf = IMG_Load_RW( rw, freeBuffer );
	if ( !tmpSurf )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to load image form memory: %s\n", IMG_GetError() );
		return NULL;
	}

	if ( formatAlpha )
	{
		formatSurf = SDL_DisplayFormatAlpha( tmpSurf );
	}
	else
	{
		formatSurf = SDL_DisplayFormat( tmpSurf );
	}
	SDL_FreeSurface( tmpSurf );

	return formatSurf;
}
//---------------------------------------
void Surface::SetColorKey( SDL_Surface* surf, unsigned int colorkeyRGB )
{
	//Uint8 a = ( colorkeyRGB              ) >> 24;
	Uint8 r = (Uint8) ( ( colorkeyRGB & 0x00FF0000 ) >> 16 );
	Uint8 g = (Uint8) ( ( colorkeyRGB & 0x0000FF00 ) >> 8  );
	Uint8 b = (Uint8) ( ( colorkeyRGB & 0x000000FF )       );

	Uint32 mappedKey = SDL_MapRGB( surf->format, r, g, b );
	SDL_SetColorKey( surf, SDL_SRCCOLORKEY, mappedKey );
}
//---------------------------------------
bool Surface::DrawSurface( SDL_Surface* surf_Dst, SDL_Surface* surf_Src, int x, int y )
{
	if ( !surf_Dst || !surf_Src )
	{
		return false;
	}

	SDL_Rect dstRect = { (Sint16) x, (Sint16) y };
	
	SDL_BlitSurface( surf_Src, NULL, surf_Dst, &dstRect );

	return true;
}
//---------------------------------------
bool Surface::DrawSurface( SDL_Surface* surf_Dst, SDL_Surface* surf_Src, int x, int y,
	int clipX, int clipY, int clipW, int clipH )
{
	if ( !surf_Dst || !surf_Src )
	{
		return false;
	}

	SDL_Rect dstRect = { (Sint16) x, (Sint16) y };
	SDL_Rect srcRect = { (Sint16) clipX, (Sint16) clipY, (Uint16) clipW, (Uint16) clipH };

	SDL_BlitSurface( surf_Src, &srcRect, surf_Dst, &dstRect );

	return true;
}
//---------------------------------------
bool Surface::DrawSurface( SDL_Surface* surf_Dst, SDL_Surface* surf_Src, int x, int y,
	int clipX, int clipY, int clipW, int clipH, float scale )
{
	SDL_Surface* surf_zoom = zoomSurface( surf_Src, scale, scale, 1 );
	DrawSurface( surf_Dst, surf_zoom, x, y, scale*clipX, scale*clipY, scale*clipW, scale*clipH );
	SDL_FreeSurface( surf_zoom );
	return true;
}
//---------------------------------------
void Surface::DrawRect( SDL_Surface* surf_Dst, int x, int y, int w, int h,
	unsigned int color, bool fill )
{
	Uint8 a = (Uint8) ( ( color              ) >> 24 );
	Uint8 r = (Uint8) ( ( color & 0x00FF0000 ) >> 16 );
	Uint8 g = (Uint8) ( ( color & 0x0000FF00 ) >> 8  );
	Uint8 b = (Uint8) ( ( color & 0x000000FF )       );

	if ( fill )
	{
		boxRGBA( surf_Dst,
			(Sint16) x, (Sint16) y,
			(Sint16) ( x + w ), (Sint16) ( y + h ),
			r, g, b, a );
	}
	else
	{
		rectangleRGBA( surf_Dst,
			(Sint16) x,
			(Sint16) y,
			(Sint16) ( x + w ), (Sint16) ( y + h ),
			r, g, b, a );
	}
}
//---------------------------------------
void Surface::DrawLine( SDL_Surface* surf_Dst, int x1, int y1, int x2, int y2,
	unsigned int color )
{
	Uint8 a = (Uint8) ( ( color              ) >> 24 );
	Uint8 r = (Uint8) ( ( color & 0x00FF0000 ) >> 16 );
	Uint8 g = (Uint8) ( ( color & 0x0000FF00 ) >> 8  );
	Uint8 b = (Uint8) ( ( color & 0x000000FF )       );

	lineRGBA( surf_Dst,
		(Sint16) x1, (Sint16) y1,
		(Sint16) x2, (Sint16) y2,
		r, g, b, a );
}
//---------------------------------------
void _DrawText( SDL_Surface* surf_Dst, int x, int y, Font* font,
	const Color& color, const char* text )
{
	font->RenderTextDynamic( surf_Dst, x, y, text, 
		color, Font::Font_ULTRA );
}
//---------------------------------------
void Surface::DrawText( SDL_Surface* surf_Dst, int x, int y, Font* font,
	const Color& color, const char* text )
{
#define DRAW_TEXT_SUB_LINE( START, END )										\
	strncpy_s( textOutputBuffer, textFormatBuffer + (START), (END) - (START) );	\
	_DrawText( surf_Dst, x, y, font, color, textOutputBuffer )

	char textFormatBuffer[ TEXT_FORMAT_BUFFER_SIZE ];
	char textOutputBuffer[ TEXT_FORMAT_BUFFER_SIZE ];

	strncpy_s( textFormatBuffer, text, strlen( text ) );

	// Scan for newlines
	int i = 0, first = 0;
	for ( ; textFormatBuffer[i]; ++i )
	{
		// Render line
		if ( textFormatBuffer[i] == '\n' )
		{
			DRAW_TEXT_SUB_LINE( first, i );

			first = i + 1;	// +1 to skip newline char
			y += font->GetRecomendedLineSpacing();
		}
		else if ( textFormatBuffer[i] == '\t' )
		{
			DRAW_TEXT_SUB_LINE( first, i );

			first = i + 1;	// +1 to skip tab char
			x += font->GetTextWidth( " " ) * 4;	// tab is assumed to be 4 spaces
		}
	}

	// Render leftovers
	if ( i != first )
	{
		DRAW_TEXT_SUB_LINE( first, i );
	}

#undef DRAW_TEXT_SUB_LINE
}
//---------------------------------------
void Surface::DrawTextFormat( SDL_Surface* surf_Dst, int x, int y, Font* font,
	const Color& color, const char* text, ... )
{
	char textFormatBuffer[ TEXT_FORMAT_BUFFER_SIZE ];

	// Apply text formating
	va_list vargs;
	va_start( vargs, text );
	vsprintf_s( textFormatBuffer, text, vargs );
	va_end( vargs );

	DrawText( surf_Dst, x, y, font, color, textFormatBuffer );
}
//---------------------------------------