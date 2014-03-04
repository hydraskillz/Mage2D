#include "MageLib.h"

using namespace mage;

//---------------------------------------
const char* GameController::GameControllerTypeToCString( int type )
{
	static const char* GameControllerTypeToCStringMapping[ GameControllerType::GC_COUNT ] =
	{
		"AND",
		"OR",
		"NAND",
		"NOR",
		"XOR",
		"XNOR",
		"EXPR",
		"SCRIPT",
	};
	if ( type >= GameController::GC_AND && type < GameController::GC_COUNT )
	{
		return GameControllerTypeToCStringMapping[ type ];
	}
	return "UNKNOWN";
}
//---------------------------------------

//---------------------------------------
// GameController
//---------------------------------------
GameController::GameController( const std::string& name, uint32 state )
	: GameLogicNode( name )
	, mState( state )
{
}
//---------------------------------------
GameController::~GameController()
{}
//---------------------------------------
void GameController::AddTrigger( GameTrigger* trigger )
{
	mTriggers.push_back( trigger );
}
//---------------------------------------
void GameController::AddAction( GameAction* action )
{
	mActions.push_back( action );
}
//---------------------------------------
int GameController::GetTypeFromString( const std::string& contTypeString )
{
	static bool _init_tables = false;
	static std::map< std::string, int > PropertyTypeToEnum;
	if ( !_init_tables )
	{
		_init_tables = true;

		PropertyTypeToEnum[ "AND"         ] = GC_AND;
		PropertyTypeToEnum[ "OR"          ] = GC_OR;
		PropertyTypeToEnum[ "NAND"        ] = GC_NAND;
		PropertyTypeToEnum[ "NOR"         ] = GC_NOR;
		PropertyTypeToEnum[ "XOR"         ] = GC_XOR;
		PropertyTypeToEnum[ "XNOR"        ] = GC_XNOR;
		PropertyTypeToEnum[ "Expression"  ] = GC_EXPR;
		PropertyTypeToEnum[ "Script"      ] = GC_SCRIPT;
	}

	return PropertyTypeToEnum[ contTypeString ];
}
//---------------------------------------
void GameController::OnUpdate()
{
	static bool was_triggered_last_call = false;
	if ( mTriggers.size() > 0 )
	{
		if ( Triggered() )
		{
			was_triggered_last_call = true;
			for ( auto action = mActions.begin(); action != mActions.end(); ++action )
			{
				(*action)->OnActivate();
			}
		}
		else if ( was_triggered_last_call )
		{
			was_triggered_last_call = false;
			for ( auto action = mActions.begin(); action != mActions.end(); ++action )
			{
				(*action)->OnDeactivate();
			}
		}
	}
}
//---------------------------------------


//---------------------------------------
// GameControllerAnd
//---------------------------------------
GameControllerAnd::GameControllerAnd( const std::string& name, uint32 state )
	: GameController( name, state )
{
	mType = GC_AND;
}
//---------------------------------------
GameControllerAnd::~GameControllerAnd()
{}
//---------------------------------------
bool GameControllerAnd::Triggered() const
{
	for ( auto trigger = mTriggers.begin(); trigger != mTriggers.end(); ++trigger )
	{
		if ( !(*trigger)->IsActive() )
		{
			return false;
		}
	}

	// One or more negative
	return true;
}
//---------------------------------------


//---------------------------------------
// GameControllerNAnd
//---------------------------------------
GameControllerNAnd::GameControllerNAnd( const std::string& name, uint32 state )
	: GameControllerAnd( name, state )
{
	mType = GC_NAND;
}
//---------------------------------------
GameControllerNAnd::~GameControllerNAnd()
{}
//---------------------------------------
bool GameControllerNAnd::Triggered() const
{
	return !GameControllerAnd::Triggered();
}
//---------------------------------------


//---------------------------------------
// GameControllerOr
//---------------------------------------
GameControllerOr::GameControllerOr( const std::string& name, uint32 state )
	: GameController( name, state )
{
	mType = GC_OR;
}
//---------------------------------------
GameControllerOr::~GameControllerOr()
{}
//---------------------------------------
bool GameControllerOr::Triggered() const
{
	for ( auto trigger = mTriggers.begin(); trigger != mTriggers.end(); ++trigger )
	{
		if ( (*trigger)->IsActive() )
		{
			return true;
		}
	}

	// All negative
	return false;
}
//---------------------------------------


//---------------------------------------
// GameControllerNOr
//---------------------------------------
GameControllerNOr::GameControllerNOr( const std::string& name, uint32 state )
	: GameControllerOr( name, state )
{
	mType = GC_NOR;
}
//---------------------------------------
GameControllerNOr::~GameControllerNOr()
{}
//---------------------------------------
bool GameControllerNOr::Triggered() const
{
	return !GameControllerOr::Triggered();
}
//---------------------------------------


//---------------------------------------
// GameControllerXOr
//---------------------------------------
GameControllerXOr::GameControllerXOr( const std::string& name, uint32 state )
	: GameController( name, state )
{
	mType = GC_XOR;
}
//---------------------------------------
GameControllerXOr::~GameControllerXOr()
{}
//---------------------------------------
bool GameControllerXOr::Triggered() const
{
	int nPos = 0;
	int nNeg = 0;
	for ( auto trigger = mTriggers.begin(); trigger != mTriggers.end(); ++trigger )
	{
		if ( (*trigger)->IsActive() )
		{
			++nPos;

			// A positive with at least 1 negative -> true
			if ( nNeg > 0 )
			{
				return true;
			}
		}
		// A negative with at least 1 positive -> true
		else if ( nPos > 0 )
		{
			return true;
		}
		else
		{
			++nNeg;
		}
	}

	// All negative or all positive
	return false;
}
//---------------------------------------


//---------------------------------------
// GameControllerXNOr
//---------------------------------------
GameControllerXNOr::GameControllerXNOr( const std::string& name, uint32 state )
	: GameControllerXOr( name, state )
{
	mType = GC_XNOR;
}
//---------------------------------------
GameControllerXNOr::~GameControllerXNOr()
{}
//---------------------------------------
bool GameControllerXNOr::Triggered() const
{
	return !GameControllerXOr::Triggered();
}
//---------------------------------------


//---------------------------------------
// GameControllerExpr
//---------------------------------------
GameControllerExpr::GameControllerExpr( const std::string& name, uint32 state,
	const std::string& expression )
	: GameController( name, state )
	, mExpr( expression )
{
	mType = GC_EXPR;
}
//---------------------------------------
GameControllerExpr::~GameControllerExpr()
{}
//---------------------------------------
bool GameControllerExpr::Triggered() const
{
	bool shouldEval = false;
	for ( auto trigger = mTriggers.begin(); trigger != mTriggers.end(); ++trigger )
	{
		if ( (*trigger)->IsActive() )
		{
			shouldEval = true;
			break;
		}
	}

	// Only evaluate the expression if an attached trigger is active
	return shouldEval && mExpr.Eval( mOwner ) != 0;
}
//---------------------------------------


//---------------------------------------
// GameControllerScript
//---------------------------------------
GameControllerScript::GameControllerScript( const std::string& name, uint32 state,
	const std::string& script )
	: GameController( name, state )
	, mScriptName( script )
{
	mType = GC_SCRIPT;	
}
//---------------------------------------
GameControllerScript::~GameControllerScript()
{}
//---------------------------------------
bool GameControllerScript::Triggered() const
{
	for ( auto trigger = mTriggers.begin(); trigger != mTriggers.end(); ++trigger )
	{
		if ( (*trigger)->IsActive() )
		{
//			PythonWrapper::RunScriptOn( mScriptName.c_str(), mOwner );
			break;
		}
	}

	// Script has full control over activation of actions
	return false;
}
//---------------------------------------