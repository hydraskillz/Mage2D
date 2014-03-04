#include "FrameRateControl.h"
#include "MageLib.h"

using namespace mage;

FrameRateControl FrameRateControl::msFPSControl;

//---------------------------------------
FrameRateControl::FrameRateControl()
	: mLastTime( 0 )
	, mFrames( 0 )
	, mFPS( 0 )
	, mTotalFrameCount( 0 )
{}
//---------------------------------------
FrameRateControl::~FrameRateControl()
{}
//---------------------------------------
FrameRateControl& FrameRateControl::GetInstance()
{
	return msFPSControl;
}
//---------------------------------------
void FrameRateControl::OnUpdate()
{
	double time = Clock::QueryTime();
	if ( mLastTime + 1.0 < time )
	{
		mLastTime = time;
		mFPS = mFrames;
		mFrames = 0;
	}

	++mFrames;
	++mTotalFrameCount;
}
//---------------------------------------