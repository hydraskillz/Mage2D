#include "MageLib.h"
#include "GameScope.h"

using namespace mage;

//---------------------------------------
GameScope::GameScope( const char* name, GameScope* parent )
	: mName( name )
	, mParentScope( parent )
	, mChildScope( NULL )
{}
//---------------------------------------
GameScope::~GameScope()
{}
//---------------------------------------