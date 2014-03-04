/*
 * Author      : Matthew Johnson
 * Date        : 26/May/2013
 * Description :
 */
 
#pragma once

// Avoid including SDL headers
struct SDL_Surface;
typedef struct _TTF_Font TTF_Font;

namespace mage
{

	class Font
	{
	public:
		static Font* DefaultFont;

		Font( const char* filename, int fontSize );
		~Font();

		enum FontQuality
		{
			Font_FAST,	// 8-bit fast quality
			Font_NICE,	// 8-bit high quality, boxed (use bg for box color) - you can colorkey it if you want.
			Font_ULTRA	// 32-bit high quality
		};
		
		// Render text to the surface dst then destroys the text object
		// Use this for text that changes frequently
		// Quality defaults to FAST
		void RenderTextDynamic( SDL_Surface* dst, int x, int y, const char* text,
			const Color& color, int fontQuality=Font_FAST, const Color& bgColor=Color::WHITE );

		// Render text to a surface, which is return for you to render
		// Use this for text that changes rarely
		// YOU are responsible for freeing the returned surface
		// Quality defaults to ULTRA
		// Returns NULL if Font failed to load or text is NULL
		SDL_Surface* RenderTextStatic( const char* text, const Color& color,
			int fontQuality=Font_ULTRA, const Color& bgColor=Color::WHITE );

		// Font info queries
		int GetRecomendedLineSpacing() const;
		int GetFontHeight() const;
		int GetTextWidth( const char* text ) const;
		void GetTextSize( const char* text, int& width, int& height ) const;

	protected:
		bool LoadFont( const char* filename, int fontSize );

	private:
		TTF_Font* mFont;
	};

}