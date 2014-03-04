#include "MageLib.h"
#include <SDL_mixer.h>

using namespace mage;

std::map< std::string, SoundClip* > SoundManager::msClips;

//---------------------------------------
bool SoundManager::OnInit()
{
	if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY,
						  MIX_DEFAULT_FORMAT,
						  MIX_DEFAULT_CHANNELS,
						  1024 ) == -1 )
	{
		return false;
	}
	int nChannels = Mix_AllocateChannels( 16 );
	ConsolePrintf( "SoundManager: Channels = %d\n", nChannels );
	return true;
}
//---------------------------------------
void SoundManager::OnExit()
{
	DestroyAllClips();
	Mix_Quit();
}
//---------------------------------------
SoundClip* SoundManager::LoadMusic( const std::string& filename )
{
	SoundClip*& clip = msClips[ filename ];

	if ( !clip )
	{
		clip = new SoundClip();

		SDL_RWops* rw;
		char* buffer=0;
		unsigned int size;
		if ( OpenDataFile( filename.c_str(), buffer, size ) == FSE_NO_ERROR )
		{
			rw = SDL_RWFromMem( buffer, size );
			clip->mData = (void*) Mix_LoadMUS_RW( rw );
			clip->mType = 1;
		}
	}
	return clip;
}
//---------------------------------------
SoundClip* SoundManager::LoadSFX( const std::string& filename )
{
	SoundClip*& clip = msClips[ filename ];

	if ( !clip )
	{
		clip = new SoundClip();

		SDL_RWops* rw;
		char* buffer=0;
		unsigned int size;
		if ( OpenDataFile( filename.c_str(), buffer, size ) == FSE_NO_ERROR )
		{
			rw = SDL_RWFromMem( buffer, size );
			clip->mData = (void*) Mix_LoadWAV_RW( rw, 1 );
			clip->mType = 0;
		}
	}
	return clip;
}
//---------------------------------------
void SoundManager::DestroyAllClips()
{
	DestroyMapByValue( msClips );
}
//---------------------------------------
bool static IsMusicPlaying()
{
	return Mix_PlayingMusic() != 0;
}
//---------------------------------------



//---------------------------------------
void SoundClip::Play( int channel, int loops )
{
	if ( mType == 0 )
	{
		//if ( mChannel != -1 && !Mix_Playing( mChannel ) )
		//{
			mChannel = Mix_PlayChannel( channel, (Mix_Chunk*) mData, loops );
			//ConsolePrintf( "Playing SFX on %d\n", mChannel );
		//}
	}
	else if ( mType == 1 )
	{
		mChannel = Mix_PlayMusic( (Mix_Music*) mData, loops );
	}
}
//---------------------------------------
void SoundClip::Stop()
{
	if ( mType == 0 )
	{
	}
	else if ( mType == 1 )
	{
		Mix_HaltMusic();
	}
}
//---------------------------------------