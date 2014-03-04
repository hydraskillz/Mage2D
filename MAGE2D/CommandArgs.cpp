#include "MageLib.h"

using namespace mage;

//---------------------------------------
CommandArgs::CommandArgs( int argc, char** argv )
{
	std::string lastCmd;
	for ( int i = 1; i < argc; ++i )
	{
		std::string arg = argv[i];
		
		if ( arg[0] == '-' )
		{
			lastCmd = arg;
			mCommandMap.insert( std::make_pair( lastCmd, std::vector< std::string >() ) );
		}
		else
		{
			mCommandMap[ lastCmd ].push_back( arg );
		}
	}
}
//---------------------------------------
CommandArgs::~CommandArgs()
{}
//---------------------------------------
bool CommandArgs::HasParam( const char* param ) const
{
	return mCommandMap.find( param ) != mCommandMap.end();
}
//---------------------------------------