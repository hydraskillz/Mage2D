/*
 * Author      : Matthew Johnson
 * Date        : 11/Jun/2013
 * Description :
 *   
 */
 
#pragma once

#define BeginCriticalSection( MUTEX )					\
	{													\
	CriticalSection MUTEX##cs( MUTEX );

#define EndCriticalSection()							\
	}

#define CriticalBlock( MUTEX )							\
	CriticalSection MUTEX##cs( MUTEX );

	

namespace mage
{

	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		// Lock the mutex
		inline void Lock();

		// Try to lock the mutex
		// This function is non-blocking
		inline bool TryLock();

		// Unlock mutex
		inline void Unlock();

	private:
		class PDIMutex* mPDIMutex;
	};
	//---------------------------------------


	//---------------------------------------
	class CriticalSection
	{
	public:
		CriticalSection( Mutex& mutex )
			: mMutex( &mutex )
		{
			mMutex->Lock();
		}

		~CriticalSection()
		{
			mMutex->Unlock();
		}

	private:
		Mutex* mMutex;
	};
	//---------------------------------------


	//---------------------------------------
	class PDIMutex
	{
	public:
		virtual ~PDIMutex() = 0;
		virtual void Lock() = 0;
		virtual bool TryLock() = 0;
		virtual void Unlock() = 0;
	};

	inline PDIMutex::~PDIMutex() {}
	//---------------------------------------


	//---------------------------------------
	inline void Mutex::Lock()
	{
		return mPDIMutex->Lock();
	}

	inline bool Mutex::TryLock()
	{
		return mPDIMutex->TryLock();
	}

	inline void Mutex::Unlock()
	{
		return mPDIMutex->Unlock();
	}
	//---------------------------------------
}