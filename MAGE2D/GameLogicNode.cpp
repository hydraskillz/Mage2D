#include "MageLib.h"

using namespace mage;

//---------------------------------------
GameLogicNode::GameLogicNode( const std::string& name )
	: mName( name )
	, mOwner( NULL )
	, mExecPriority( 0 )
{}
//---------------------------------------
GameLogicNode::~GameLogicNode()
{}
//---------------------------------------
void GameLogicNode::AttachTo( GameObject* newOwner, int execPriority )
{
	mOwner = newOwner;
	mExecPriority = execPriority;
}
//---------------------------------------
std::string GameLogicNode::EvaluateVariablePropertyString( const std::string& var )
{
	return mOwner->EvaluateVariablePropertyString( var );
	/*
	std::string result = var;
	if ( var.length() )
	{
		if ( var.at( 0 ) == '@' )
		{
			GameVar& gvar = mOwner->GetProperty( var.substr( 1 ) );
			if ( gvar.GetType() == GV_STRING )
			{
				result = EvaluateVariablePropertyString( gvar.ValueString() );
			}
			else
			{
				ConsolePrintf( CONSOLE_WARNING,
					"%s\n"					\
					" objectName = %s\n"	\
					"              ^\n"		\
					" variable must be of type string\n"
					, mName.c_str()
					, var.c_str() );
			}
		}
	}
	return result;*/
}
//---------------------------------------