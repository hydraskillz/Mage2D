#include "MageLib.h"

using namespace mage;

//---------------------------------------
// Constant Color Definitions
//---------------------------------------
const Color Color::RED           ( 0xFFFF0000 );
const Color Color::GREEN         ( 0xFF00FF00 );
const Color Color::BLUE          ( 0xFF0000FF );
const Color Color::DARK_RED      ( 0xFF800000 );
const Color Color::DARK_GREEN    ( 0xFF008000 );
const Color Color::DARK_BLUE     ( 0xFF0000A0 );
const Color Color::LIGHT_BLUE    ( 0xFFADD8E6 );
const Color Color::YELLOW        ( 0xFFFFFF00 );
const Color Color::DARK_YELLOW   ( 0xFF808000 );
const Color Color::CYAN          ( 0xFF00FFFF );
const Color Color::ORANGE        ( 0xFFFFA500 );
const Color Color::PURPLE        ( 0xFF800080 );
const Color Color::PINK          ( 0xFFFF00FF );
const Color Color::GREY          ( 0xFF808080 );
const Color Color::LIGHT_GREY    ( 0xFFE0E0E0 );
const Color Color::WHITE         ( 0xFFFFFFFF );
const Color Color::BLACK         ( 0xFF000000 );
const Color Color::TRANSPARENCY  ( 0x00000000 );

//---------------------------------------
Color::Color()
	: bgra( 0 )
{}
//---------------------------------------
Color::Color( uint32 argb )
	: bgra( argb )
{}
//---------------------------------------
Color::Color( uint8 a, uint8 r, uint8 g, uint8 b )
	: a( a )
	, r( r )
	, g( g )
	, b( b )
{}
//---------------------------------------