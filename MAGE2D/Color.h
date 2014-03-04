/*
 * Author      : Matthew Johnson
 * Date        : 26/May/2013
 * Description :
 *   32-bit Color
 *   Constructors take ARGB, but storage is BGRA due to endianness.
 *   A union is used to store BGRA and allow easy access to individual components.
 */
 
#pragma once

namespace mage
{

	class Color
	{
	public:
		Color();
		Color( uint32 argb );
		Color( uint8 a, uint8 r, uint8 g, uint8 b );

#pragma warning( push )
#pragma warning( disable : 4201 )
		union
		{
			uint32 bgra;
			struct
			{
				uint8 b, g, r, a;
			};
		};
#pragma warning( pop )

		// Color constants
		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;
		static const Color DARK_RED;
		static const Color DARK_GREEN;
		static const Color DARK_BLUE;
		static const Color LIGHT_BLUE;
		static const Color YELLOW;
		static const Color DARK_YELLOW;
		static const Color CYAN;
		static const Color ORANGE;
		static const Color PURPLE;
		static const Color PINK;
		static const Color WHITE;
		static const Color GREY;
		static const Color LIGHT_GREY;
		static const Color BLACK;
		static const Color TRANSPARENCY;
	};

}