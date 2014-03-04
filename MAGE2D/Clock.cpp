#include "MageLib.h"

using namespace mage;

Clock* Clock::msMasterClock = nullptr;
const double Clock::DEFAULT_MAX_DELTA_SECONDS = 1.0 / 60.0;

#ifdef WIN32
#	include <Windows.h>

//---------------------------------------
double GetInverseFrequency()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency( &freq );
	return 1.0 / (double) freq.QuadPart;
}
//---------------------------------------

class ClockWin32
	: public ClockPDI
{
public:
	//---------------------------------------
	ClockWin32()
		: mIsQueryTime( false )
		, mStartTime( 0.0 )
		, mEndTime( 0.0 )
	{
		mInverseFrequency = GetInverseFrequency();
		mStartTick.QuadPart = 0;
		mEndTick.QuadPart   = 0;
	}
	//---------------------------------------
	void BeginTimeQuery()
	{
		mIsQueryTime = true;
		QueryPerformanceCounter( &mStartTick );
		mStartTime = mStartTick.QuadPart * mInverseFrequency;
	}
	//---------------------------------------
	void EndTimeQuery()
	{
		mIsQueryTime = false;
		QueryPerformanceCounter( &mEndTick );
	}
	//---------------------------------------
	double QueryTimeElapsed( Clock::TimeFormat timeFormat )
	{
		if ( mIsQueryTime )
		{
			QueryPerformanceCounter( &mEndTick );
		}

		mEndTime = mEndTick.QuadPart * mInverseFrequency;

		double timeSec = mEndTime - mStartTime;

		return Clock::GetFormatedTime( timeFormat, timeSec );
	}
	//---------------------------------------
	static double QueryTime( Clock::TimeFormat timeFormat )
	{
		LARGE_INTEGER time;
		static double invFreq = GetInverseFrequency();

		QueryPerformanceCounter( &time );

		return Clock::GetFormatedTime( timeFormat, (double) time.QuadPart * invFreq );
	}
	//---------------------------------------
private:
	bool mIsQueryTime;
	double mStartTime;
	double mEndTime;

	double mInverseFrequency;
	LARGE_INTEGER mStartTick;
	LARGE_INTEGER mEndTick;

};

#endif

//---------------------------------------
Clock::Clock()
	: mElapsedSeconds( 0.0 )
	, mDeltaSeconds( 0.0 )
	, mMaxDeltaSeconds( DEFAULT_MAX_DELTA_SECONDS )
	, mTimeScale( 1.0 )
	, mIsPaused( false )
	, mParent( nullptr )
{
#ifdef WIN32
	mClockPDI = new ClockWin32();
#endif
}
//---------------------------------------
Clock::~Clock()
{
	delete mClockPDI;
}
//---------------------------------------
void Clock::SetTimeScale( double timeScale )
{
	mTimeScale = timeScale;
}
//---------------------------------------
void Clock::Pause()
{
	mIsPaused = true;
}
//---------------------------------------
void Clock::Resume()
{
	mIsPaused = false;
}
//---------------------------------------
void Clock::AdvanceTime( double deltaSeconds )
{
	// No delta if paused
	if ( mIsPaused )
	{
		deltaSeconds = 0.0;
	}

	// Limit max delta
	if ( deltaSeconds > mMaxDeltaSeconds )
	{
		deltaSeconds = mMaxDeltaSeconds;
	}

	// Advance time
	mDeltaSeconds = deltaSeconds * mTimeScale;
	mElapsedSeconds += mDeltaSeconds;

	// Check for events that need fired
	// Note: Cannot use iterators since container may be modified during an event callback.
	int numEvents = mTimeEvents.size();
	//for ( auto itr = mTimeEvents.begin(); itr != mTimeEvents.end(); ++itr )
	for ( int i = 0; i < numEvents; ++i )
	{
		if ( mTimeEvents[i].TimeToFireSeconds <= mElapsedSeconds )
		{
			EventManager::FireEvent( mTimeEvents[i].EventName );
		}
		// Since events are sorted by time, no other events will fire
		else	
		{
			break;
		}
	}

	// Remove fired events
	mTimeEvents.erase( std::remove_if( mTimeEvents.begin(), mTimeEvents.end(),
		[&]( const ClockTimeEventCallback& ev )
	{
		return ev.TimeToFireSeconds <= mElapsedSeconds;
	}), mTimeEvents.end() );

	// Advance children
	for ( auto clockItr = mChildren.begin(); clockItr != mChildren.end(); ++clockItr )
	{
		(*clockItr)->AdvanceTime( deltaSeconds );
	}
}
//---------------------------------------
void Clock::SetMaxDeltaSeconds( double deltaSeconds )
{
	mMaxDeltaSeconds = deltaSeconds;
}
//---------------------------------------
void Clock::SetTime( double timeSeconds )
{
	mElapsedSeconds = timeSeconds;
}
//---------------------------------------
void Clock::PostEventCallbackAt( const std::string& eventName, double timeToFireSecods )
{
	ClockTimeEventCallback ctec = { eventName, timeToFireSecods };
	mTimeEvents.push_back( ctec );
	std::sort( mTimeEvents.begin(), mTimeEvents.end() );
}
//---------------------------------------
void Clock::PostEventCallbackAfter( const std::string& eventName, double delaySeconds )
{
	PostEventCallbackAt( eventName, mElapsedSeconds + delaySeconds );
}
//---------------------------------------
void Clock::BeginTimeQuery()
{
	mClockPDI->BeginTimeQuery();
}
//---------------------------------------
void Clock::EndTimeQuery()
{
	mClockPDI->EndTimeQuery();
}
//---------------------------------------
double Clock::QueryTimeElapsed( TimeFormat timeFormat )
{
	return mClockPDI->QueryTimeElapsed( timeFormat );
}
//---------------------------------------
double Clock::QueryTime( TimeFormat timeFormat )
{
#ifdef WIN32
	return ClockWin32::QueryTime( timeFormat );
#endif
}
//---------------------------------------
double Clock::GetFormatedTime( TimeFormat timeFormat, double time )
{
	switch ( timeFormat )
	{
	case TIME_SEC:   return time;
	case TIME_MILLI: return time * 0.001;
	case TIME_MICRO: return time * 0.000001;
	default:		 assertion( false, "Invalid TimeForamt '%d'.\n", timeFormat ); return 0.0;
	}
}
//---------------------------------------


//---------------------------------------
// Factory functions
//---------------------------------------
Clock& Clock::Initialize()
{
	if ( msMasterClock == nullptr )
	{
		msMasterClock = New0 Clock;
	}

	return *msMasterClock;
}
//---------------------------------------
Clock* Clock::CreateClock( Clock* parent )
{
	Clock* clock = New0 Clock;
	clock->mParent = parent;
	parent->mChildren.push_back( clock );
	return clock;
}
//---------------------------------------
void Clock::DestroyClock( Clock*& clock )
{
	if ( !clock ) return;
	// Destroying a child clock
	if ( clock->mParent )
	{
		auto clockItr = std::find( clock->mParent->mChildren.begin(),
			clock->mParent->mChildren.end(), clock );
		if ( clockItr != clock->mParent->mChildren.end() )
		{
			clock->mParent->mChildren.erase( clockItr,
				clock->mParent->mChildren.end() );
			Delete0( clock );
		}
	}
	// Destroying the root clock
	else if ( clock == msMasterClock )
	{
		Delete0( msMasterClock );
	}
}
//---------------------------------------