#include "MageLib.h"

#include <Windows.h>

using namespace mage;

//---------------------------------------
class MutexWin32
	: public PDIMutex
{
public:
	//---------------------------------------
	MutexWin32()
		: mHasKey( true )
	{
		InitializeCriticalSection( &mCS );
	}
	//---------------------------------------
	virtual ~MutexWin32()
	{
		DeleteCriticalSection( &mCS );
	}
	//---------------------------------------
	void Lock()
	{
		EnterCriticalSection( &mCS );
		while ( !mHasKey ) Sleep( 1000 );
		mHasKey = false;
	}
	//---------------------------------------
	bool TryLock()
	{
		bool canEnter = TryEnterCriticalSection( &mCS ) == 0 ? false : true;
		if ( canEnter && !mHasKey )
		{
			LeaveCriticalSection( &mCS );
			canEnter = false;
		}
		return canEnter;
	}
	//---------------------------------------
	void Unlock()
	{
		mHasKey = true;
		LeaveCriticalSection( &mCS );
	}
	//---------------------------------------
private:
	CRITICAL_SECTION mCS;
	bool mHasKey;

};
//---------------------------------------


//---------------------------------------
Mutex::Mutex()
	: mPDIMutex( new MutexWin32 )
{}
//---------------------------------------
Mutex::~Mutex()
{
	delete mPDIMutex;
}
//---------------------------------------