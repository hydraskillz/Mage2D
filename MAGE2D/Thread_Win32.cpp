#include "MageLib.h"

#include <Windows.h>
#include <process.h>
#include <exception>

using namespace mage;

//---------------------------------------
// Info passed to wrapper function
struct ThreadInfo
{
	Thread::Function Function;
	void *Arg;
	class ThreadWin32* TheThread;
};
//---------------------------------------


//---------------------------------------
class ThreadWin32
	: public PDIThread
{
public:
	//---------------------------------------
	ThreadWin32( Thread::Function function, void* userData, unsigned int stackSize=0 )
	{
		ThreadInfo* info = new ThreadInfo;
		info->Function = function;
		info->Arg = userData;
		info->TheThread = this;

		mAlive = true;

		mHandle = (HANDLE) _beginthreadex( 
			0,
			stackSize,
			_thread_wrapper_function,
			(void*) info,
			0,
			&mThreadId );

		// Failed to create thread
		if ( !mHandle )
		{
			mAlive = false;
			delete info;
		}
	}
	//---------------------------------------
	virtual ~ThreadWin32()
	{
		if ( Joinable() )
		{
			std::terminate();
		}
	}
	//---------------------------------------
	void Join()
	{
		if ( Joinable() )
		{
			WaitForSingleObject( mHandle, INFINITE );
			CloseHandle( mHandle );
		}
	}
	//---------------------------------------
	bool Joinable() const
	{
		bool joinable;

		mMutex.Lock();
		joinable = mAlive;
		mMutex.Unlock();

		return joinable;
	}
	//---------------------------------------
	unsigned int GetThreadId() const
	{
		return mThreadId;
	}
	//---------------------------------------
private:
	static unsigned int WINAPI _thread_wrapper_function( void* arg );

	HANDLE mHandle;
	unsigned int mThreadId;
	mutable Mutex mMutex;
	bool mAlive;
};

//---------------------------------------
// Thread wrapper function
unsigned int WINAPI ThreadWin32::_thread_wrapper_function( void* arg )
{
	ThreadInfo* info = (ThreadInfo*) arg;

	try
	{
		info->Function( info->Arg );
	}
	catch ( ... )
	{
		std::terminate();
	}

	info->TheThread->mMutex.Lock();
	info->TheThread->mAlive = false;
	info->TheThread->mMutex.Unlock();

	delete info;

	return 0;
}
//---------------------------------------


//---------------------------------------
Thread::Thread( Function function, void* userData, unsigned int stackSize )
	: mPDIThread( new ThreadWin32( function, userData, stackSize ) )
{}
//---------------------------------------
Thread::~Thread()
{
	delete mPDIThread;
}
//---------------------------------------
void Thread::Join()
{
	mPDIThread->Join();
}
//---------------------------------------
bool Thread::Joinable()
{
	return mPDIThread->Joinable();
}
//---------------------------------------
void Thread::Sleep( unsigned long ms )
{
	::Sleep( ms );
}
//---------------------------------------
unsigned int Thread::GetMaxThreadConcurrency()
{
	SYSTEM_INFO si;
	GetSystemInfo( &si );
	return (unsigned int) si.dwNumberOfProcessors;
}
//---------------------------------------