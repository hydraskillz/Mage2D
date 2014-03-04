/*
 * Author      : Matthew Johnson
 * Date        : 11/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	//---------------------------------------
	class Thread
	{
	public:
		typedef void (*Function)( void* );

		Thread( Function function, void* userData, unsigned int stackSize=0 );
		~Thread();

		void Join();
		bool Joinable();
		inline unsigned int GetThreadId() const;

		static void Sleep( unsigned long ms );
		static unsigned int GetMaxThreadConcurrency();

	private:
		class PDIThread* mPDIThread;
	};
	//---------------------------------------


	//---------------------------------------
	class PDIThread
	{
	public:
		virtual ~PDIThread() = 0;
		virtual void Join() = 0;
		virtual bool Joinable() const = 0;
		//virtual void Sleep( unsigned long ms ) = 0;
		virtual unsigned int GetThreadId() const = 0;
	};

	inline PDIThread::~PDIThread() {}
	//---------------------------------------


	//---------------------------------------
	inline unsigned int Thread::GetThreadId() const
	{
		return mPDIThread->GetThreadId();
	}
	//---------------------------------------

}