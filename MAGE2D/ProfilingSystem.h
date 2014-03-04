/*
 * Author      : Matthew Johnson
 * Date        : 3/Jun/2013
 * Description :
 *   
 */
 
#pragma once

#define PROFILING_ENABLED

#ifdef PROFILING_ENABLED
#	define ProfileSection( TAG )												\
	static int TAG##PROFILER_ID = ProfilingSystem::GetIdFromName( #TAG );		\
	Profiler p( TAG##PROFILER_ID );											

#	define BeginProfilingSection( TAG )											\
		static int TAG##PROFILER_ID = ProfilingSystem::GetIdFromName( #TAG );	\
		{																		\
		Profiler p( TAG##PROFILER_ID );											

#	define EndProfilingSection( TAG )											\
		}
#else
#	define BeginProfilingSection( TAG )
#	define EndProfilingSection( TAG )
#endif

namespace mage
{

	class Profiler
	{
	public:
		Profiler( int id );
		~Profiler();

		int mId;
		double mStartTimeSeconds;
	};

	struct ProfilingData
	{
		ProfilingData( const std::string& tag );

		std::string Tag;
		double TotalTimeSeconds;
		double TotalFrameTimeSeconds;
		double AverageFrameTimeSeconds;
		double MaxTimeEver;
		int Count;
		int CountPerFrame;

		double GetAverageTime() const;
	};

	class ProfilingSystem
	{
		ProfilingSystem();
	public:
		~ProfilingSystem();

		static void EndFrame();
		static int GetIdFromName( const std::string& tag );
		static ProfilingData& GetDataFromId( int id );
		static const std::vector< ProfilingData >& GetProfilerData();

	private:
		static std::vector< ProfilingData > msProfilerData;
		static std::map< std::string, int > msTagToIdMap;
	};

}