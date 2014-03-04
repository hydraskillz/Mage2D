#include "MageLib.h"

#ifndef MAGE_USE_IN_GAME_CONSOLE
#include <Windows.h>

using namespace mage;

HANDLE console = GetStdHandle( STD_OUTPUT_HANDLE );
void SetConsoleStyle( uint16 style )
{
	SetConsoleTextAttribute( console, style );
}
#endif