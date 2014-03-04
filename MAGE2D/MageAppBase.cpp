#include "MageLib.h"
#include "MageAppBase.h"

using namespace mage;

MageAppBase* MageAppBase::TheGame = 0;
MageAppBase::EntryPoint MageAppBase::Run = 0;

//---------------------------------------
MageAppBase::MageAppBase()
{}
//---------------------------------------
MageAppBase::~MageAppBase()
{}
//---------------------------------------
int main( int argc, char** argv )
{
	int exitCode = 0;

	// Initialize memory
#ifdef USE_MEMORY_MANAGER
	if ( !MemoryPool::InitializeMemory() )
	{
		return -1;
	}
#endif

	// Initialize everything
	InitTerm::RunInitializers();

	try
	{
		// Run the app
		exitCode = MageAppBase::Run( argc, argv );
	}
	catch ( std::exception e )
	{
		assertion( false, "An exception has occured:\n\n%s\n", e.what() );
	}

	// Clean up everything
	InitTerm::RunTerminators();

	// Free memory
#ifdef USE_MEMORY_MANAGER
	MemoryPool::TerminateMemory();
#endif

	return exitCode;
}
//---------------------------------------