#include "MageLib.h"

#include <SDL.h>
#include <SDL_ttf.h>

using namespace mage;

static InputManager gInputManager;

//---------------------------------------
MageGame::MageGame( const char* title, int windowPositionX,
	int windowPositionY, int windowWidth, int windowHeight,
	bool fullscreen )
	: mTitle( title )
	, mWindowWidth( windowWidth )
	, mWindowHeight( windowHeight )
	, mWindowPosX( windowPositionX )
	, mWindowPosY( windowPositionY )
	, mFullscreen( fullscreen )
	, mRunning( true )
	, mFrameTime( 0.0 )
	, mLimitFrameRate( true )
	, mDisplayProfilingInfo( false )
	, mCamera( NULL )
	, mDataRoot( "../data/" )
	, mFileSystemDataPreference( DATA_FILESYSTEM_FIRST )
	, mProfilerX( 20 )
	, mProfilerY( 20 )
{}
//---------------------------------------
MageGame::~MageGame()
{}
//---------------------------------------
bool MageGame::_OnInitialize()
{
	// Initialize SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		assertion( false, "SDL failed to init!\n%s\n", SDL_GetError() );
		return false;
	}

	// Initialize TTF
	if ( TTF_Init() < 0 )
	{
		assertion( false, "TTF failed to init!\n%s\n", TTF_GetError() );
		return false;
	}

	// Center the window
	SDL_putenv( "SDL_VIDEO_CENTERED=center" );
	// Set the video flags
	Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	// Fullscreen or not
	if ( mFullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}
	// Create the window and framebuffer
	mDisplaySurface = SDL_SetVideoMode( mWindowWidth, mWindowHeight, 32, flags );
	
	if ( !mDisplaySurface )
	{
		assertion( false, "Failed to create window!\n%s", SDL_GetError() );
		return false;
	}

	// Set key repeat
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

	// Register event system callbacks
	RegisterEventCallbacks();

	// Set file system variables
	SetFileSystemPriority( (FileSystemDataPriority) mFileSystemDataPreference );

	// Create console (do this as soon as possible)
	ConsoleCreate( mWindowWidth, mWindowHeight * 0.5f );

	// Load the default font
	mDefaultFont = new Font( "fonts/ConsolaMono.ttf", 13 );
	Font::DefaultFont = mDefaultFont;

	// Initialize console (needs to happen after a font initialization)
	ConsoleInit();

	SetConsoleDefaultStyle( C_FG_LIGHT_WHITE | C_BG_BLACK );

	REGISTER_CONSOLE_COMMAND( "ShowDebugLogic",[&]( const std::string& s )
	{
		mEditor.SetActiveDefinition( GameObjectDefinition::GetDefinitionByName( s ) );
	}, "", "" );

	// Setup Job Manager
	JobManager::CreateJobManager();

	// Initialize sound system
	SoundManager::OnInit();

	// Load data files
	LoadData();

	// Set title and icon
	SDL_WM_SetCaption( mTitle, NULL );

	// Create the clock
	mClock = &Clock::Initialize();

	// Initialize Python
//	PythonWrapper::Initialize();

	// @TODO removeme Test script
//	PythonWrapper::RunScript( "../data/script/test.py" );
	
	return OnInitialize();
}
//---------------------------------------
void MageGame::OnUpdate()
{ /* Implemented by derived class */ }
//---------------------------------------
void MageGame::_OnUpdate()
{
	ProfileSection( Update );

	const float dt = (float) mClock->GetDeltaTime( Clock::TIME_SEC );

	// Frame rate profiling
	FrameRateControl::GetInstance().OnUpdate();

	// Send out input events
	gInputManager.OnUpdate();

	// Send out job completion callbacks
	JobManager::GetInstance()->OnUpdate();

	// Update animations
	SpriteManager::Manager.OnUpdate( dt );

	// Update map/entities
	mGameMap.OnUpdate( dt );

	GameInfo& gameInfo = GameInfo::GetGameInfo();

	if ( gameInfo.ShouldLoadLevel )
	{
		LoadLevel( gameInfo.CurrentLevel.c_str() );
		gameInfo.ShouldLoadLevel = false;
	}

	// User updates
	OnUpdate();
}
//---------------------------------------
void MageGame::OnDisplay()
{
	ProfileSection( Renderer );

	SDL_Rect scrn = { 0, 0, (Uint16) mWindowWidth, (Uint16) mWindowHeight };
	SDL_FillRect( mDisplaySurface, &scrn, 0x000000 );

	// Render the active map
	mGameMap.OnDraw( mDisplaySurface, *mCamera );

	// @TODO sprites may be on different layers...
	// maybe map should store sprites in its layers
	// and control sprite drawing??
	SpriteManager::Manager.OnDraw( mDisplaySurface, *mCamera );

	// User overlay
	OnDisplayOverlay();

	mEditor.OnDraw( mDisplaySurface );

	// Console
	ConsoleRender( (void*) mDisplaySurface );
}
//---------------------------------------
void MageGame::OnDisplayOverlay()
{}
//---------------------------------------
void MageGame::DisplayProfilingInfo()
{
	const std::vector< ProfilingData >& data = ProfilingSystem::GetProfilerData();
	std::string output = "=Profiler=\n";

	int maxTagLen = 0;
	for ( auto itr = data.begin(); itr != data.end(); ++itr )
	{
		int len = itr->Tag.length();
		if ( len > maxTagLen ) maxTagLen = len;
	}

	int i = 1;
	int lineLen;
	for ( auto itr = data.begin(); itr != data.end(); ++itr )
	{
		char buff[256];
		lineLen = sprintf_s( buff, " %*s  Avg(ms) %7.3f  Max(ms) %7.3f\n",
			-maxTagLen,
			itr->Tag.c_str(), 
			itr->AverageFrameTimeSeconds * 1000,
			itr->MaxTimeEver * 100 );

		output += buff;

		// Draw some lines in between rows of text
		lineLen *= 7;
		Surface::DrawLine( mDisplaySurface, 
			mProfilerX, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
			mProfilerX + lineLen, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
			Color::GREY.bgra );
		++i;
	}
	// Draw bottom line
	Surface::DrawLine( mDisplaySurface, 
		mProfilerX, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
		mProfilerX + lineLen, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
		Color::GREY.bgra );

	// Vertical lines
	Surface::DrawLine( mDisplaySurface, 
		mProfilerX + (maxTagLen + 19) * 7, mProfilerY + mDefaultFont->GetRecomendedLineSpacing(),
		mProfilerX + (maxTagLen + 19) * 7, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
		Color::GREY.bgra );

	Surface::DrawLine( mDisplaySurface, 
		mProfilerX + lineLen, mProfilerY + mDefaultFont->GetRecomendedLineSpacing(),
		mProfilerX + lineLen, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
		Color::GREY.bgra );

	Surface::DrawLine( mDisplaySurface, 
		mProfilerX, mProfilerY + mDefaultFont->GetRecomendedLineSpacing(),
		mProfilerX, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
		Color::GREY.bgra );

	Surface::DrawLine( mDisplaySurface, 
		mProfilerX + (maxTagLen + 1) * 7, mProfilerY + mDefaultFont->GetRecomendedLineSpacing(),
		mProfilerX + (maxTagLen + 1) * 7, mProfilerY + mDefaultFont->GetRecomendedLineSpacing() * i,
		Color::GREY.bgra );

	// Text shadow
	Surface::DrawTextFormat( mDisplaySurface, mProfilerX + 1, mProfilerY + 1,
		mDefaultFont, Color::BLACK, output.c_str() );

	// Text
	Surface::DrawTextFormat( mDisplaySurface, mProfilerX, mProfilerY,
		mDefaultFont, Color::WHITE, output.c_str() );
}
//---------------------------------------
void MageGame::OnTerminate()
{ /* Implemented by derived class */ }
//---------------------------------------
void MageGame::_OnTerminate()
{
	OnTerminate();

	// Quit SDL (display surface is freed by SDL_Quit)
	SDL_Quit();

	// Quit all jobs
	JobManager::DestroyJobManager();

	// Shut down Python
//	PythonWrapper::ShutDown();
}
//---------------------------------------
void MageGame::SpawnSpriteEvent( Dictionary& params )
{
	std::string spriteName;
	Vec2f position;

	// Get name
	if ( params.Get( "SpriteName", spriteName ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn sprite: missing SpriteName\n" );
		return;
	}

	// Get position
	if ( params.Get( "Position", position ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn sprite: missing Position\n" );
		return;
	}

	// Spawn sprite
	SpawnSprite( spriteName, position );
}
//---------------------------------------
void MageGame::SpawnEntityEvent( Dictionary& params )
{
	std::string entityName;
	Vec2f position;

	// Get name
	if ( params.Get( "EntityName", entityName ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn entity: missing EntityName\n" );
		return;
	}

	// Get position
	if ( params.Get( "Position", position ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn entity: missing Position\n" );
		return;
	}

	// Spawn entity
	Entity* entity = SpawnEntity( entityName, position );
	params.Set( "Entity", entity );
}
//---------------------------------------
void MageGame::SpawnGameObjectEvent( Dictionary& params )
{
	std::string entityName;
	Vec2f position;

	// Get name
	if ( params.Get( "ObjectName", entityName ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn GameObject: missing ObjectName\n" );
		return;
	}

	// Get position
	if ( params.Get( "Position", position ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to spawn GameObject: missing Position\n" );
		return;
	}

	// Spawn entity
	GameObject* gameObject = SpawnGameObject( entityName, position );
	params.Set( "GameObject", gameObject );
}
//---------------------------------------
GameObject* MageGame::SpawnGameObject( const std::string& name, const Vec2f& position )
{
	GameObjectDefinition* entityDef = GameObjectDefinition::GetDefinitionByName( name );
	if ( entityDef )
	{
		GameObject* entity = entityDef->Create();
		entity->Position = position;
		//entity->SetGameMap( &mGameMap );
		mGameObjects.push_back( entity );
		return entity;
	}
	ConsolePrintf( CONSOLE_ERROR, "Failed to spawn GameObject: No GameObject found '%s'\n", name.c_str() );
	return NULL;
}
//---------------------------------------
void MageGame::SetCameraBoundsEvent( Dictionary& params )
{
	RectI bounds;

	if ( params.Get( "Bounds", bounds ) != Dictionary::DErr_SUCCESS )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to set camera bounds: missing Bounds\n" );
		return;
	}

	mCamera->SetWorldBounds( bounds );
}
//---------------------------------------
void MageGame::SpawnSprite( const std::string& name, const Vec2f& position )
{
	SpriteManager::Manager.CreateSprite( name, position );
}
//---------------------------------------
Entity* MageGame::SpawnEntity( const std::string& name, const Vec2f& position )
{
	EntityDefinition* entityDef = EntityDefinition::GetDefinitionByName( name );
	if ( entityDef )
	{
		Entity& entity = entityDef->Create( position );
		entity.SetGameMap( &mGameMap );
		return &entity;
	}
	ConsolePrintf( CONSOLE_ERROR, "Failed to spawn Entity: No Entity found '%s'\n", name.c_str() );
	return NULL;
}
//---------------------------------------
int MageGame::Run( int argc, char** argv )
{
	MageAppBase* theGame = ( MageAppBase* ) TheGame;
	return theGame->Main( argc, argv );
}
//---------------------------------------
void MageGame::ProcessEvent( SDL_Event* sdlEvent )
{
	switch ( sdlEvent->type )
	{
		case SDL_ACTIVEEVENT:
		{
			break;
		}

		case SDL_KEYDOWN:
		{
			if ( sdlEvent->key.keysym.sym == SDLK_ESCAPE )
			{
				OnExit();
			}
			else if ( !ConsoleInput( sdlEvent->key.keysym.sym, sdlEvent->key.keysym.mod ) )
			{
				gInputManager.SetKeyDown( sdlEvent->key.keysym.sym );
			}
			break;
		}

		case  SDL_KEYUP:
		{
			if ( sdlEvent->key.keysym.sym == SDLK_f )
			{
				mLimitFrameRate = !mLimitFrameRate;
			}

			else if ( sdlEvent->key.keysym.sym == SDLK_F7 )
			{
				mDisplayProfilingInfo = !mDisplayProfilingInfo;
			}

			else if ( sdlEvent->key.keysym.sym == SDLK_F8 )
			{
				mGameMap.ToggleObjectDebug();
			}

			else if ( sdlEvent->key.keysym.sym == SDLK_F5 )
			{
				ConsoleClear();
				LoadData();
			}
			else
			{
				gInputManager.SetKeyUp( sdlEvent->key.keysym.sym );
			}

			break;
		}
		case SDL_MOUSEMOTION:
		{
			mMousePosX = sdlEvent->motion.x;
			mMousePosY = sdlEvent->motion.y;
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			if ( IsConsoleVisible() )
			{
				switch ( sdlEvent->button.button )
				{
					case SDL_BUTTON_WHEELUP:
					{
						ConsoleScroll( 1 );
						break;
					}
					case SDL_BUTTON_WHEELDOWN:
					{
						ConsoleScroll( -1 );
						break;
					}
				}
			}
			else
			{
				switch ( sdlEvent->button.button )
				{
					case SDL_BUTTON_WHEELUP:
					{
						int c = 2;
						if ( sdlEvent->button.x < mWindowWidth / 3.0f )
							c = 0;
						else if ( sdlEvent->button.x < 2 * mWindowWidth / 3.0f )
							c = 1;
						mEditor.Scroll( 1, c );
						break;
					}
					case SDL_BUTTON_WHEELDOWN:
					{
						int c = 2;
						if ( sdlEvent->button.x < mWindowWidth / 3.0f )
							c = 0;
						else if ( sdlEvent->button.x < 2 * mWindowWidth / 3.0f )
							c = 1;
						mEditor.Scroll( -1, c );
						break;
					}
				}
			}
			break;
		}

		case SDL_QUIT:
		{
			OnExit();
			break;
		}
	}
}
//---------------------------------------
void MageGame::RegisterEventCallbacks()
{
	EventManager::RegisterObjectForEvent( "SpawnSprite", *this, &MageGame::SpawnSpriteEvent );
	EventManager::RegisterObjectForEvent( "SpawnEntity", *this, &MageGame::SpawnEntityEvent );
	//EventManager::RegisterObjectForEvent( "SpawnGameObject", *this, &MageGame::SpawnGameObjectEvent );
	EventManager::RegisterObjectForEvent( "SetCameraBounds", *this, &MageGame::SetCameraBoundsEvent );
}
//---------------------------------------
void MageGame::LoadData()
{
	// Load GameInfo
	GameInfo::CreateGameInfo( "GameInfo.xml" );
	GameInfo& gameInfo = GameInfo::GetGameInfo();

	if ( gameInfo.GameName.size() != 0 )
	{
		mTitle = gameInfo.GameName.c_str();
	}
	
	// Load input bindings
	gInputManager.LoadKeyCodes( "KeyCodes.xml" );
	gInputManager.LoadInputBindings( "InputBindings.xml" );
	
	// Load entity definitions
	std::vector< std::string > filenames;
	std::string dir = mDataRoot + "sprites/";
	/*GetAllFilesByExtension( dir.c_str(), "*.entity", filenames );

	SpriteManager::Manager.DestroyAllSprites();
	EntityDefinition::DestroyAllDefinitions();

	for ( auto itr = filenames.begin(); itr != filenames.end(); ++itr )
	{
		new EntityDefinition( std::string( "sprites/" ) + *itr );
	}*/

	GameTriggerManager::DestroyAllTriggers();
	GameActionManager::DestroyAllActions();
	GameObjectDefinition::DestroyAllDefinitions();

	// Load GameObject definition files
	filenames.clear();

	// Engine data files
	dir = "../data/objects/";
	GetAllFilesByExtension( dir.c_str(), "*.xml", filenames );

	// User data files
	dir = mDataRoot + "objects/";
	GetAllFilesByExtension( dir.c_str(), "*.xml", filenames );

	for ( auto itr = filenames.begin(); itr != filenames.end(); ++itr )
	{
		new GameObjectDefinition( std::string( "objects/" ) + *itr );
	}

	//mEditor.SetActiveDefinition( GameObjectDefinition::GetDefinitionByName( "Player" ) );

	//ConsolePrintf( "Expr = %d\n", GameExpression( "(52*(((9+8)*(4*6))+7))" ).Eval() );
	//ConsolePrintf( "Expr = %d\n", GameExpression( "(4 + 5) == 0" ).Eval() );
	//ConsolePrintf( "Expr = %d\n", GameExpression( "(4 + @prop1) == 0" ).Eval() );

	//SoundManager::OnInit();
	//SoundClip* clip = SoundManager::LoadSFX( "../MarioClone/data/sfx/smb_1-up.wav" );
	//clip->Play( );
	
	// Create a main camera
	Delete0( mCamera );
	mCamera = new Camera( mWindowWidth, mWindowHeight );
	mGameMap.SetCamera( mCamera );

	// Load startup map, if available
	if ( gameInfo.StartUpMap.size() )
	{
		LoadLevel( gameInfo.StartUpMap.c_str() );
	}
}
//---------------------------------------
void MageGame::LoadLevel( const char* filename )
{
	if ( !mGameMap.Load( filename ) )
	{
		ConsolePrintf( CONSOLE_WARNING, "Unable to load map '%s'. File not found\n", filename );
	}
}
//---------------------------------------
bool MageGame::ExecuteCommadLine( CommandArgs* cmdArgs )
{
	bool continueExecution = true;

	// Fullscreen flag
	if ( cmdArgs->HasParam( "-fullscreen" ) )
	{
		mFullscreen = true;
	}

	// Window size
	if ( cmdArgs->HasParam( "-winsize" ) )
	{
		int sx=0, sy=0;
		cmdArgs->GetArgAs( "-winsize", sx, 0 );
		cmdArgs->GetArgAs( "-winsize", sy, 1 );

		ConsolePrintf( "Setting window size to %d x %d\n", sx, sy );
	}

	// Generate files command
	if ( cmdArgs->HasParam( "-generateFiles" ) )
	{
		int fileCount, fileSize;

		if ( !cmdArgs->GetArgAs( "-generateFiles", fileCount, 0 ) )
		{
			ConsolePrintf( CONSOLE_ERROR, "Usage: -generateFiles fileCount fileSize\n" );
		}

		if ( !cmdArgs->GetArgAs( "-generateFiles", fileSize, 1 ) )
		{
			ConsolePrintf( CONSOLE_ERROR, "Usage: -generateFiles fileCount fileSize\n" );
		}

		GenerateRandomAssFiles( "../data", fileCount, fileSize );

		continueExecution = false;
	}

	// Set file loading preference
	if ( cmdArgs->HasParam( "-fileLoadPreference" ) )
	{
		std::string pref;
		if ( !cmdArgs->GetArgAs( "-fileLoadPreference", pref, 0 ) )
		{
			ConsolePrintf( CONSOLE_ERROR, "Usage: -fileLoadPreference PREFERENCE\n FILESYSTEM_ONLY\n ARCHIVE_ONLY\n FILESYSTEM_FIRST (default)\n ARCHIVE_FIRST\n" );
		}

		if ( pref == "FILESYSTEM_ONLY" )
		{
			mFileSystemDataPreference = DATA_FILESYSTEM_ONLY;
		}
		else if ( pref == "ARCHIVE_ONLY" )
		{
			mFileSystemDataPreference = DATA_ARCHIVE_ONLY;
		}
		else if ( pref == "FILESYSTEM_FIRST" )
		{
			mFileSystemDataPreference = DATA_FILESYSTEM_FIRST;
		}
		else if ( pref == "ARCHIVE_FIRST" )
		{
			mFileSystemDataPreference = DATA_ARCHIVE_FIRST;
		}
		else
		{
			ConsolePrintf( CONSOLE_ERROR, "Usage: -fileLoadPreference PREFERENCE\n FILESYSTEM_ONLY\n ARCHIVE_ONLY\n FILESYSTEM_FIRST (default)\n ARCHIVE_FIRST\n" );
		}
	}

	return continueExecution;
}
//---------------------------------------
void MageGame::OnExit()
{
	mRunning = false;
}
//---------------------------------------
int MageGame::Main( int argc, char** argv )
{
	//MageAppBase* theGame = ( MageAppBase* ) TheGame;

	CommandArgs cmdArgs( argc, argv );

	// Run commandlet
	if ( !ExecuteCommadLine( &cmdArgs ) )
	{
		return 0;
	}

	// Initial game
	if ( !_OnInitialize() )
	{
		return -1;
	}

	SDL_Event sdlEvent;

	while ( mRunning )
	{
		ProfilingSystem::EndFrame();

		mClock->BeginTimeQuery();
		
		while ( SDL_PollEvent( &sdlEvent ) )
		{
			ProcessEvent( &sdlEvent );
		}

		_OnUpdate();
		OnDisplay();

		// Delay if we finish frame early
		mFrameTime = mClock->QueryTimeElapsed( Clock::TIME_SEC );
		if ( mLimitFrameRate && mFrameTime < 1.0 / 60.0 )
		{
			SDL_Delay( (Uint32) ( 1000.0 * ( 1.0 / 60.0 - mClock->QueryTimeElapsed( Clock::TIME_MILLI ) ) ) );
		}
		mClock->EndTimeQuery();
		mFrameTime = mClock->QueryTimeElapsed( Clock::TIME_SEC );
		mClock->AdvanceTime( mFrameTime );

		if ( mDisplayProfilingInfo )
		{
			DisplayProfilingInfo();
		}

		SDL_Flip( mDisplaySurface );
	}

	_OnTerminate();

	return 0;
}
//---------------------------------------