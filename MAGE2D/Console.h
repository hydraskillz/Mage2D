/*
 * Author      : Matthew Johnson
 * Date        : 7/Jun/2013
 * Description :
 *   
 */
 
#pragma once

// Foreground colors
#define C_FG_BLACK          0x00
#define C_FG_BLUE           0x01
#define C_FG_GREEN          0x02
#define C_FG_AQUA           0x03
#define C_FG_RED            0x04
#define C_FG_PURPLE         0x05
#define C_FG_YELLOW         0x06
#define C_FG_WHITE          0x07
#define C_FG_GRAY           0x08
#define C_FG_GREY           0x08
#define C_FG_LIGHT_BLUE     0x09
#define C_FG_LIGHT_GREEN    0x0A
#define C_FG_LIGHT_AQUA     0x0B
#define C_FG_LIGHT_RED      0x0C
#define C_FG_LIGHT_PURPLE   0x0D
#define C_FG_LIGHT_YELLOW   0x0E
#define C_FG_LIGHT_WHITE    0x0F

// Background colors
#define C_BG_BLACK          0x00
#define C_BG_BLUE           0x10
#define C_BG_GREEN          0x20
#define C_BG_AQUA           0x30
#define C_BG_RED            0x40
#define C_BG_PURPLE         0x50
#define C_BG_YELLOW         0x60
#define C_BG_WHITE          0x70
#define C_BG_GRAY           0x80
#define C_BG_GREY           0x80
#define C_BG_LIGHT_BLUE     0x90
#define C_BG_LIGHT_GREEN    0xA0
#define C_BG_LIGHT_AQUA     0xB0
#define C_BG_LIGHT_RED      0xC0
#define C_BG_LIGHT_PURPLE   0xD0
#define C_BG_LIGHT_YELLOW   0xE0
#define C_BG_LIGHT_WHITE    0xF0

// Predefined styles
#define CONSOLE_WARNING C_FG_LIGHT_YELLOW | C_BG_BLACK
#define CONSOLE_ERROR C_FG_LIGHT_RED | C_BG_BLACK
#define CONSOLE_SUCCESS C_FG_LIGHT_GREEN | C_BG_BLACK
#define CONSOLE_INFO C_FG_AQUA | C_BG_BLACK
#define CONSOLE_DEFAULT -1

//#define MAGE_USE_IN_GAME_CONSOLE

void SetConsoleDefaultStyle( mage::uint16 style );
void SetConsoleStyle( mage::uint16 style );
void ConsolePrintf( mage::uint16 style, const char* fmt, ... );
void ConsolePrintf( const char* fmt, ... );
void ConsoleClear();
bool ConsoleInit();
void ConsoleCreate( float width, float height );
void ConsoleRender( void* screen );
//void ConsoleToggleVisibility();
bool IsConsoleVisible();
void ConsoleScroll( int dir );
bool ConsoleInput( int code, int mod );
void* GetGameConsole();

#ifndef MAGE_USE_IN_GAME_CONSOLE
#	define REGISTER_CONSOLE_COMMAND( name, fn, help, args )
#else
	struct SDL_Surface;
	namespace mage
	{
	class CommandLine
	{
	public:
		typedef std::function< void( const std::string& args ) > CommandFn;

		CommandLine( float width, float height );
		~CommandLine();

		bool Initialize();

		static void RegisterCommand( const std::string& name,
			CommandFn fn, const std::string& help="",
			const std::string& argsInfo="" );
		static void Exe( const std::string& cmd );
		void Input( bool* keystate, bool* specialstate );

		const std::string& GetText() const;

		struct AutoCompleteLine
		{
			std::string line;
			int matchyness;
			bool operator<( const AutoCompleteLine& other )
			{
				return matchyness > other.matchyness;
			}
		};
		const std::vector< AutoCompleteLine >& GetAutoCompleteLines() const;

		float GetCursorPos() const;
		float GetBaseHeight() const;

		int GetMaxAutoCompleteLines() const;

		bool IsVisible() const { return mVisible; }
		void ToggleVisibility() { mVisible = !mVisible; }
		void SetVisible( bool visible ) { mVisible = visible; }

		void SetPositionY( float y ) { mY = y; }

		static void Clear( const std::string& args );
		static void Help( const std::string& args );
		static void Exit( const std::string& args );
		static void CmdFireEvent( const std::string& args );

		bool OnKeyDown( int key, int mod );
		bool OnKeyJustDown( int key, int mod );

		void OnDraw( SDL_Surface* screen ) const;

	private:
		bool ParseKey( int key, int mod );
		// Insert character at cursor
		void InsertCharacter( char c );
		void ShiftCursor( bool left );
		void CursorHome();
		void CursorEnd();
		void Delete( bool rightOfCursor );
		void NewHistory();
		void OldHistory();

		void FillFromAutoComplete();
		void UpdateAutoComplete();

		struct CmdMapping
		{
			CmdMapping( const std::string& name, CommandFn func,
				const std::string& help="",
				const std::string& argsInfo="" )
				: FnName( name )
				, Fn( func )
				, FnHelp( help )
				, FnArgsInfo( argsInfo )
			{}
			const std::string FnName;
			const std::string FnHelp;
			const std::string FnArgsInfo;
			CommandFn Fn;
		};
		typedef std::pair< std::string, CommandFn > FnMapping;

		static std::map< std::string, CmdMapping* > mCommands;

		class Font* mFont;
		float mWidth, mHeight;
		bool mVisible;

		std::string mCmdLine;
		std::vector< AutoCompleteLine > mAutoCompleteLines;
		int mCursorPos;
		std::vector< std::string >  mHistory;
		int mHistoryPointer;
		float mOriginalHeight;
		float mOriginalY;
		float mY;
		int mMaxAutoCompleteLines;
	};
#	define REGISTER_CONSOLE_COMMAND( name, fn, help, args )		\
{ ((CommandLine*)GetGameConsole())->RegisterCommand( name, fn, help, args ); }
}
#endif