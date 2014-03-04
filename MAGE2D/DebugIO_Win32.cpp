#include "MageLib.h"

#include <Windows.h>

using namespace mage;

extern "C" void OutputDebugMessage( const char* fmt, ... )
{
	char msg[ 1024 ];
	va_list vargs;
	va_start( vargs, fmt );
	vsprintf_s( msg, fmt, vargs );
	va_end( vargs );
	OutputDebugString( msg );
}