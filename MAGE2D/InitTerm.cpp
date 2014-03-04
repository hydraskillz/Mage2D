#include "MageLib.h"

using namespace mage;

int InitTerm::msInitializerCount = 0;
InitTerm::Initializer InitTerm::msIntializers[ MAX_INITTERM ];
int InitTerm::msTerminatorCount = 0;
InitTerm::Terminator InitTerm::msTerminators[ MAX_INITTERM ];

//---------------------------------------
void InitTerm::AddInitializer( Initializer function )
{
	if ( msInitializerCount < MAX_INITTERM )
	{
		msIntializers[ msInitializerCount++ ] = function;
	}
	else
	{
		assertion( false, "MAX_INITTERM(%d) exceeded! Increase this value and rebuild Core.\n", MAX_INITTERM );
	}
}
//---------------------------------------
void InitTerm::RunInitializers()
{
	for ( int i = 0; i < msInitializerCount; ++i )
	{
		msIntializers[i]();
	}
}
//---------------------------------------
void InitTerm::AddTerminator( Terminator function )
{
	if ( msTerminatorCount < MAX_INITTERM )
	{
		msTerminators[ msTerminatorCount++ ] = function;
	}
	else
	{
		assertion( false, "MAX_INITTERM(%d) exceeded! Increase this value and rebuild Core.\n", MAX_INITTERM );
	}
}
//---------------------------------------
void InitTerm::RunTerminators()
{
	for ( int i = 0; i < msTerminatorCount; ++i )
	{
		msTerminators[i]();
	}
}
//---------------------------------------