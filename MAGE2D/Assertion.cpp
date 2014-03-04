#include "MageLib.h"

#ifdef MAGE_USE_ASSERTION

#include <Windows.h>
#include <stdio.h>

using namespace mage;

const char* Assertion::msDebugPrompt = "\nDebug?";
const size_t Assertion::msDebugPromptLength = strlen( Assertion::msDebugPrompt );
const char* Assertion::msMessagePrefix = "\nAssertion failure at %s(%d):\n";
const char* Assertion::msMessageBoxTitle = "Assertion failure";

//---------------------------------------
Assertion::Assertion( bool condition, const char* file, int line, const char* format, ... )
{
	if ( condition )
	{
		return;
	}

	char message[ MAX_MSG_SIZE ];
	const size_t maxPrefixSize = MAX_MSG_SIZE - msDebugPromptLength - 1;
	_snprintf_s( message, maxPrefixSize, msMessagePrefix, file, line );

	va_list args;
	va_start( args, format );
	size_t length = strlen( message );
	_vsnprintf_s( message + length, MAX_MSG_SIZE - length, maxPrefixSize - length, format, args );
	va_end( args );

	strcat_s( message, msDebugPrompt );
	OutputDebugStringA( message );
	int selection = ::MessageBox( 0, message, msMessageBoxTitle,
		MB_ICONERROR | MB_YESNOCANCEL | MB_APPLMODAL | MB_TOPMOST );

	switch ( selection )
	{
	case IDYES:
		// Break and debug
		__debugbreak();
		break;
	case IDNO:
		// Continue;
		break;
	case IDCANCEL:
	default:
		// Terminate
		exit( 0 );
		break;
	}
}
//---------------------------------------
Assertion::~Assertion()
{}
//---------------------------------------
#endif