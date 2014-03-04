/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   SDL based 2D game engine.
 */
 
#pragma once

#include "MageLib.h"

struct SDL_Surface;
union SDL_Event;

namespace mage
{

	class MageGame
		: public MageAppBase
	{
	protected:
		MageGame( const char* title, int windowPositionX,
			int windowPositionY, int windowWidth, int windowHeight,
			bool fullscreen = false );
	public:
		virtual ~MageGame();

		// Called on game first start
		virtual bool OnInitialize() = 0;
		// Called once per frame, before rendering
		virtual void OnUpdate();
		// Called from OnDisplay() after the level has been drawn
		virtual void OnDisplayOverlay();
		// Called when the game is exiting.
		virtual void OnTerminate();

		// I/O events
		virtual void OnExit();

		// Entry point
		virtual int Main( int argc, char** argv );

		// Events
		void SpawnSpriteEvent( Dictionary& params );
		void SpawnEntityEvent( Dictionary& params );
		void SetCameraBoundsEvent( Dictionary& params );
		void SpawnGameObjectEvent( Dictionary& params );

		// Spawn a sprite at the give location (world space)
		void SpawnSprite( const std::string& name, const Vec2f& position );
		class Entity* SpawnEntity( const std::string& name, const Vec2f& position );
		GameObject* SpawnGameObject( const std::string& name, const Vec2f& position );

	protected:
		// Hook to main
		static int Run( int argc, char** argv );

		const char* mTitle;
		int mWindowWidth, mWindowHeight;
		int mWindowPosX, mWindowPosY;
		bool mFullscreen;
		bool mRunning;

		bool mDisplayProfilingInfo;

		SDL_Surface* mDisplaySurface;
		Map mGameMap;
		Camera* mCamera;
		Font* mDefaultFont;
		Clock* mClock;
		double mFrameTime;
		bool mLimitFrameRate;
		int mFileSystemDataPreference;
		int mProfilerX, mProfilerY;
		int mMousePosX, mMousePosY;
		GameNodeEditor mEditor;

		std::string mDataRoot;

		std::vector< GameObject* > mGameObjects;

	private:
		void ProcessEvent( SDL_Event* sdlEvent );
		void RegisterEventCallbacks();
		void LoadData();
		void LoadLevel( const char* filename );
		bool ExecuteCommadLine( class CommandArgs* cmdArgs );

		void _OnUpdate();
		void OnDisplay();
		void DisplayProfilingInfo();
		bool _OnInitialize();
		void _OnTerminate();
	};

//---------------------------------------
// Macros
//---------------------------------------
#define MAGE_GAME_APP( classname ) \
	MAGE_IMP_INIT( classname ) \
	MAGE_IMP_TERM( classname ) \
	void classname::Initialize() \
	{ \
		MageAppBase::Run = &MageGame::Run; \
		TheGame = New0 classname(); \
	} \
	void classname::Terminate() \
	{ \
		Delete0( TheGame ); \
	}
//---------------------------------------

}