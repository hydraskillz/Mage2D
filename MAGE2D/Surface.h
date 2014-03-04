/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   Basically a 2D render/texture class hybrid.
 */
 
#pragma once

// Avoid including SDL headers
struct SDL_Surface;

namespace mage
{

	class Surface
	{
	protected:
		Surface();
	public:
		//---------------------------------------
		// Utility Functions

		// Load an image to a SDL_Surface
		static SDL_Surface* LoadSurface( const char* filename, bool formatAlpha=true );
		// Same as LoadSurface but loads from memory.
		static SDL_Surface* LoadSurfaceMem( void* imgBuffer, int bufferSize, int freeBuffer=1, bool formatAlpha=true );
		static void SetColorKey( SDL_Surface* surf, unsigned int colorkeyRGB );

		//---------------------------------------


		//---------------------------------------
		// Texture Rendering

		// Draw a surface onto another surface
		static bool DrawSurface( SDL_Surface* surf_Dst, SDL_Surface* surf_Src, int x, int y );
		// Draw surface using clipping
		static bool DrawSurface( SDL_Surface* surf_Dst, SDL_Surface* surf_Src, int x, int y,
			int clipX, int clipY, int clipW, int clipH );
		// Draw surface using clipping with uniform scaling @warn slow as hell
		static bool DrawSurface( SDL_Surface* surf_Dst, SDL_Surface* surf_Src, int x, int y,
			int clipX, int clipY, int clipW, int clipH, float scale );

		//---------------------------------------


		//---------------------------------------
		// Primitive Rendering

		// Draw a rectangle of the given color
		static void DrawRect( SDL_Surface* surf_Dst, int x, int y, int w, int h,
			unsigned int color, bool fill=true );
		// Draw a line from (x1, y1) to (x2, y2)
		static void DrawLine( SDL_Surface* surf_Dst, int x1, int y1, int x2, int y2,
			unsigned int color );

		//---------------------------------------


		//---------------------------------------
		// Text Rendering

	private:
		static const size_t TEXT_FORMAT_BUFFER_SIZE = 1024U;

	public:
		// Draw text (dynamic text)
		static void DrawText( SDL_Surface* surf_Dst, int x, int y, Font* font,
			const Color& color, const char* text );
		// Draw text (dynamic text) with formating
		static void DrawTextFormat( SDL_Surface* surf_Dst, int x, int y, Font* font,
			const Color& color, const char* text, ... );

		//---------------------------------------

	};

}