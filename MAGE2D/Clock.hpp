//---------------------------------------
inline double Clock::GetElapsedTime( TimeFormat timeFormat ) const
{
	return GetFormatedTime( timeFormat, mElapsedSeconds );
}
//---------------------------------------
inline double Clock::GetDeltaTime( TimeFormat timeFormat ) const
{
	double scaledDT = mDeltaSeconds * mTimeScale;
	return GetFormatedTime( timeFormat, scaledDT );
}
//---------------------------------------
inline double Clock::GetTimeScale() const
{
	return mTimeScale;
}
//---------------------------------------
inline bool Clock::IsPaused() const
{
	return mIsPaused;
}
//---------------------------------------