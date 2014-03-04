/*
 * Author      : Matthew Johnson
 * Date        : 7/Jul/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class SoundClip
	{
		friend class SoundManager;
	public:
		void Play( int channel=-1, int loops=0 );
		void Stop();
	protected:
		int mChannel;
		int mType;
		void* mData;
	};

	class SoundManager
	{
	public:
		SoundManager();
		~SoundManager();

		static bool OnInit();
		static void OnExit();

		static SoundClip* LoadMusic( const std::string& filename );
		static SoundClip* LoadSFX( const std::string& filename );

		static void DestroyAllClips();

		static bool IsMusicPlaying();

	private:
		static std::map< std::string, SoundClip* > msClips;

	};

}