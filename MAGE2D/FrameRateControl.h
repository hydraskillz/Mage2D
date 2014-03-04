/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class FrameRateControl
	{
	protected:
		FrameRateControl();
	public:
		~FrameRateControl();

		static FrameRateControl& GetInstance();

		void OnUpdate();
		inline int GetFPS() const;
		inline long GetTotalFrames() const;

	private:
		// Singleton
		static FrameRateControl msFPSControl;

		double mLastTime;
		int mFrames;
		int mFPS;
		long mTotalFrameCount;

	};

#include "FrameRateControl.hpp"

}