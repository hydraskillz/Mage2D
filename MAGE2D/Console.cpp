#include "MageLib.h"

using namespace mage;

static uint16 DEFAULT_STYLE = C_FG_WHITE | C_BG_BLACK;
static Mutex gConsoleMutex;

#ifdef MAGE_USE_IN_GAME_CONSOLE
#include <SDL.h>
namespace mage
{

	//---------------------------------------
	static class Console* gConsole;
	static class CommandLine* gCommandLine;
	//---------------------------------------

	

	// In-game console
	class Console
	{
	public:

		struct ConsoleLine
		{
			ConsoleLine() : Text( "" ), TextColor( Color::WHITE ) {}
			ConsoleLine( const char* text, const Color& color )
				: Text( text ), TextColor( color ) {}
			
			std::string Text;
			Color TextColor;
		};

		Console( uint32 bufferSize, float width, float height );
		~Console();

		bool Initialize();

		void Printf( const char* fmt, ... );
		void Printf( const Color& color, const char* fmt, ... );
		void VPrintf( const Color& color, const char* fmt, va_list vargs );
		void Clear();
		void Scroll( int dir );
		int GetLineBegin() const;
		int GetLineEnd() const;

		int GetOffset() const;

		int GetNumLines() const;
		int GetMaxLines() const;
		int GetMaxVisibileLines() const;
		const ConsoleLine& GetLine( int i ) const;

		bool IsVisible() const;
		void ToggleVisibility();
		void SetVisible( bool visible ) { mVisible = visible; }

		void OnDraw( SDL_Surface* screen ) const;

//	private:
		void Console::Push( const char* text, const Color& color );

		float mWidth, mHeight;
		int mOffset;
		Font* mConsoleFont;
		float mConsoleFontScale;

		int mMaxLines; // Max lines that are visible at once
		int mBufferSize;
		bool mVisible;

		Color mTextColor;

		CircularBuffer< ConsoleLine > mLines;
	};

	//---------------------------------------
	Console::Console( uint32 bufferSize, float width, float height )
		: mWidth( width )
		, mHeight( height )
		, mOffset( 0 )
		, mMaxLines( 0 )
		, mBufferSize( bufferSize )
		, mVisible( false )
	{
		mLines.Resize( bufferSize );
	}
	//---------------------------------------
	Console::~Console()
	{}
	//---------------------------------------
	bool Console::Initialize()
	{
		mConsoleFont = Font::DefaultFont;
		mMaxLines = (int) ( mHeight / mConsoleFont->GetFontHeight() );

		return true;
	}
	//---------------------------------------
	void Console::Printf( const char* fmt, ... )
	{
		va_list vargs;
		va_start( vargs, fmt );
		VPrintf( Color::WHITE, fmt, vargs );
		va_end( vargs );
	}
	//---------------------------------------
	void Console::Printf( const Color& color, const char* fmt, ... )
	{
		va_list vargs;
		va_start( vargs, fmt );
		VPrintf( color, fmt, vargs );
		va_end( vargs );
	}
	//---------------------------------------
	void Console::VPrintf( const Color& color, const char* fmt, va_list vargs )
	{
		char buffer[256];
		vsprintf_s( buffer, fmt, vargs );
		Push( buffer, color );
	}
	//---------------------------------------
	void Console::Clear()
	{
		mLines.Clear();
		mOffset = 0;
	}
	//---------------------------------------
	void Console::Scroll( int dir )
	{
		// Don't scroll if text isn't overflowing
		if ( mLines.Length() < mMaxLines ) return;

		mOffset += dir;

		// Clamp offsets
		if ( mOffset < 0 )
		{
			mOffset = 0;
		}
		if ( mOffset > mLines.Length() - mMaxLines )
		{
			mOffset = mLines.Length() - mMaxLines;
		}
	}
	//---------------------------------------
	int Console::GetLineBegin() const
	{
		return mOffset;
	}
	//---------------------------------------
	int Console::GetLineEnd() const
	{
		return Mathi::Min( mMaxLines, mLines.Length() ) + mOffset;
	}
	//---------------------------------------
	int Console::GetOffset() const
	{
		return mOffset;
	}
	//---------------------------------------
	int Console::GetNumLines() const
	{
		return mLines.Length();
	}
	//---------------------------------------
	int Console::GetMaxLines() const
	{
		return mBufferSize;
	}
	//---------------------------------------
	int Console::GetMaxVisibileLines() const
	{
		return mMaxLines;
	}
	//---------------------------------------
	const Console::ConsoleLine& Console::GetLine( int i ) const
	{
		return mLines.Read( i );
	}
	//---------------------------------------
	void Console::Push( const char* text, const Color& color )
	{
		mLines.Write( ConsoleLine( text, color ) );
		if ( mLines.Length() > mMaxLines )
		{
			mOffset = mLines.Length() - mMaxLines;
		}
	}
	//---------------------------------------
	bool Console::IsVisible() const
	{
		return mVisible;
	}
	//---------------------------------------
	void Console::ToggleVisibility()
	{
		mVisible = !mVisible;
	}
	//---------------------------------------
	void Console::OnDraw( SDL_Surface* screen ) const
	{
		if ( !mVisible ) return;

		// Background
		Surface::DrawRect( screen, 0, 0, (int) mWidth,   (int) mHeight, Color::BLACK.bgra );
		Surface::DrawRect( screen, 0, 0, (int) mWidth-1, (int) mHeight, Color::GREEN.bgra, false );

		// Text
		const int start = GetLineBegin();
		const int end = GetLineEnd();
		int y = 0;
		int x = 6;
		for ( int i = start; i < end; ++i )
		{
			const ConsoleLine& line = GetLine( i );

			Surface::DrawTextFormat( screen, x, y, mConsoleFont, line.TextColor, line.Text.c_str() );
			y += mConsoleFont->GetRecomendedLineSpacing();
		}

		// Scroll bar
		int totalLines = GetNumLines();
		int maxVisibleLines = GetMaxVisibileLines();
		if ( totalLines > maxVisibleLines )
		{
			float barHeight = 10;
			float barStart = GetOffset() / (float) ( totalLines - maxVisibleLines ) * mHeight;

			y = 1;
			x = 1;

			barStart += y;

			Surface::DrawRect( screen, x, y, 2, (int) mHeight - 1, Color::DARK_GREEN.bgra );
			Surface::DrawRect( screen, x, (int) Mathf::Clamp( barStart, (float) y, mHeight - 1 - barHeight ),
				2, (int) barHeight, Color::GREEN.bgra );
		}
	}
	//---------------------------------------


	// CommandLine used with in-game console
/*	class CommandLine
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

		bool OnKeyDown( int key );
		bool OnKeyJustDown( int key );

		void OnDraw( SDL_Surface* screen ) const;

	private:
		bool ParseKey( int key );
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

		Font* mFont;
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
	*/
	std::map< std::string, CommandLine::CmdMapping* > CommandLine::mCommands;

	//---------------------------------------
	CommandLine::CommandLine( float width, float height )
		: mWidth( width )
		, mHeight( height )
		, mCursorPos( 0 )
		, mHistoryPointer( -1 )
		, mFont( nullptr )
		, mVisible( false )
	{
		RegisterCommand( "Clear",        Clear,            "Clear the console."   );
		RegisterCommand( "Help",         Help,             "Display help dialog." );
		RegisterCommand( "Exit",         Exit,             "Exit the App."        );
		RegisterCommand( "Quit",         Exit,             "Exit the App."        );
		RegisterCommand( "FireEvent",    CmdFireEvent,     "Fire a named event.", "sEventName [params - varies with event]" );

		mOriginalHeight = mHeight;
		mY = 0;
		mOriginalY = mY;
		mMaxAutoCompleteLines = 5;
	}
	//---------------------------------------
	CommandLine::~CommandLine()
	{
		DestroyMapByValue( mCommands );
	}
	//---------------------------------------
	bool CommandLine::Initialize()
	{
		mFont = Font::DefaultFont;
		return true;
	}
	//---------------------------------------
	void CommandLine::RegisterCommand( const std::string& name, CommandFn fn,
		const std::string& help, const std::string& argsInfo )
	{
		mCommands[ StringUtil::StringToLower( name ) ] = new CmdMapping( name, fn, help, argsInfo );
	}
	//---------------------------------------
	void CommandLine::Exe( const std::string& cmd )
	{
		std::string fn;
		std::string args;

		int s = cmd.find_first_of( ' ' );
		if ( s == std::string::npos )
		{
			fn = cmd;
			args = "";
		}
		else
		{
			fn = cmd.substr( 0, s );
			args = cmd.substr( s+1, cmd.length() - s );
		}

		StringUtil::StringToLower( fn );

		for ( auto itr = mCommands.begin(); itr != mCommands.end(); ++ itr )
		{
			if ( fn == itr->first )
			{
				( itr->second->Fn )( args );
				return;
			}
		}

		ConsolePrintf( CONSOLE_ERROR, "Command '%s' not found!", fn.c_str() );
	}
	//---------------------------------------
	const std::string& CommandLine::GetText() const
	{
		return mCmdLine;
	}
	//---------------------------------------
	const std::vector< CommandLine::AutoCompleteLine >& CommandLine::GetAutoCompleteLines() const
	{
		return mAutoCompleteLines;
	}
	//---------------------------------------
	float CommandLine::GetCursorPos() const
	{
		return (float) ( 6 + mFont->GetTextWidth( mCmdLine.substr( 0, mCursorPos ).c_str() ) );
	}
	//---------------------------------------
	float CommandLine::GetBaseHeight() const
	{
		return mOriginalHeight;
	}
	//---------------------------------------
	int CommandLine::GetMaxAutoCompleteLines() const
	{
		return mMaxAutoCompleteLines;
	}
	//---------------------------------------
	bool CommandLine::OnKeyDown( int key, int mod )
	{
		return ParseKey( key, mod );
	}
	//---------------------------------------
	bool CommandLine::OnKeyJustDown( int key, int mod )
	{
		if ( key == '`' )
		{
			if ( mVisible && !IsConsoleVisible() )
			{
				SetPositionY( mOriginalY );
				gConsole->ToggleVisibility();
			}
			else
			{
				mOriginalY = mY;
				ToggleVisibility();
				gConsole->ToggleVisibility();
			}
			return true;
		}

		if ( key == SDLK_TAB )
		{
			if ( !mVisible && !IsConsoleVisible() )
			{
				ToggleVisibility();
				mOriginalY = mY;
				SetPositionY( 0 );
				return true;
			}
		}

		if ( IsVisible() && key == SDLK_ESCAPE )
		{
			SetPositionY( mOriginalY );
			SetVisible( false );
			gConsole->SetVisible( false );
			return true;
		}

		return ParseKey( key, mod );
	}
	//---------------------------------------
	bool CommandLine::ParseKey( int key, int mod )
	{
		if ( !IsVisible() ) return false;

		// Printable character
		if ( key >= 32 && key <= 125 )
		{
			if ( mod & KMOD_LSHIFT )
				InsertCharacter( ::toupper( key ) );
			else
				InsertCharacter( key );
			return true;
		}

		// Left arrow
		if ( key == SDLK_LEFT )
		{
			ShiftCursor( true );
			return true;
		}
		// Right arrow
		if ( key == SDLK_RIGHT )
		{
			ShiftCursor( false );
			return true;
		}

		// Up arrow
		if ( key == SDLK_UP )
		{
			NewHistory();
			return true;
		}
		// Down arrow
		if ( key == SDLK_DOWN )
		{
			OldHistory();
			return true;
		}

		// Home
		if ( key == SDLK_HOME )
		{
			CursorHome();
			return true;
		}
		// End
		if ( key == SDLK_END )
		{
			CursorEnd();
			return true;
		}

		// Backspace
		if ( key == SDLK_BACKSPACE )
		{
			Delete( false );
			return true;
		}
		// Delete
		if ( key == SDLK_DELETE )
		{
			Delete( true );
			return true;
		}

		// Enter
		if ( key == SDLK_RETURN )
		{
			if ( mCmdLine.length() )
			{
				Exe( mCmdLine );
				mHistory.push_back( mCmdLine );
				mCmdLine = "";
				mCursorPos = 0;
				UpdateAutoComplete();
			}
			// Close console if Enter pressed with empty line
			else
			{
				SetPositionY( mOriginalY );
				gConsole->SetVisible( false );
				SetVisible( false );
			}
			return true;
		}

		// Tab
		if ( key == SDLK_TAB )
		{
			FillFromAutoComplete();
			return true;
		}

		return false;
	}
	//---------------------------------------
	void CommandLine::InsertCharacter( char c )
	{
		if ( mCursorPos == mCmdLine.length() )
		{
			mCmdLine += c;
		}
		else
		{
			mCmdLine = mCmdLine.substr( 0, mCursorPos ) + c + mCmdLine.substr( mCursorPos );
		}
		UpdateAutoComplete();
		++mCursorPos;
	}
	//---------------------------------------
	void CommandLine::ShiftCursor( bool left )
	{
		if ( left )
		{
			--mCursorPos;
			mCursorPos = mCursorPos < 0 ? 0 : mCursorPos;
		}
		// Right
		else
		{
			++mCursorPos;
			mCursorPos = mCursorPos > (int) mCmdLine.length() ? (int) mCmdLine.length() : mCursorPos;
		}
	}
	//---------------------------------------
	void CommandLine::CursorHome()
	{
		mCursorPos = 0;
	}
	//---------------------------------------
	void CommandLine::CursorEnd()
	{
		mCursorPos = mCmdLine.length();
	}
	//---------------------------------------
	void CommandLine::Delete( bool rightOfCursor )
	{
		if ( rightOfCursor )
		{
			// Make sure there is something to delete
			if ( mCmdLine.length() && mCursorPos < (int) mCmdLine.length() )
			{
				mCmdLine = mCmdLine.substr( 0, mCursorPos ) + mCmdLine.substr( mCursorPos + 1 );
			}
		}
		// Left of cursor
		else
		{
			// Make sure there is something to delete
			if ( mCmdLine.length() && mCursorPos > 0 )
			{
				mCmdLine = mCmdLine.substr( 0, mCursorPos - 1 ) + mCmdLine.substr( mCursorPos );
				--mCursorPos;
			}
		}
		UpdateAutoComplete();
	}
	//---------------------------------------
	void CommandLine::NewHistory()
	{

		if ( mHistory.size() > 0 )
		{
			// Newest history (wrap to back of vector)
			if ( mHistoryPointer == -1 )
			{
				mHistoryPointer = mHistory.size() - 1;
				mCmdLine = mHistory[ mHistoryPointer ];
			}
			// Next history
			else if ( mHistoryPointer - 1 >= 0 )
			{
				mCmdLine = mHistory[ --mHistoryPointer ];
			}
			// The present (blank line)
			else
			{
				mHistoryPointer = -1;
				mCmdLine = "";
			}
			mCursorPos = mCmdLine.length();
			UpdateAutoComplete();
		}
	}
	//---------------------------------------
	void CommandLine::OldHistory()
	{
		if ( mHistory.size() > 0 )
		{
			// Oldest history (wrap to front of vector)
			if ( mHistoryPointer == -1 )
			{
				mHistoryPointer = 0;
				mCmdLine = mHistory[ mHistoryPointer ];
			}
			// Next history
			else if ( mHistoryPointer + 1 < (int) mHistory.size() )
			{
				mCmdLine = mHistory[ ++mHistoryPointer ];
			}
			// The present (blank line)
			else
			{
				mHistoryPointer = -1;
				mCmdLine = "";
			}
			mCursorPos = mCmdLine.length();
			UpdateAutoComplete();
		}
	}
	//---------------------------------------
	void CommandLine::FillFromAutoComplete()
	{
		if ( mAutoCompleteLines.size() > 0 )
		{
			mCmdLine = mAutoCompleteLines[0].line;
			auto cmdFound = mCommands.find( mCmdLine );
			if ( cmdFound != mCommands.end() )
			{
				mCmdLine = cmdFound->second->FnName;
			}
			UpdateAutoComplete();
			CursorEnd();
		}
	}
	//---------------------------------------
	void CommandLine::UpdateAutoComplete()
	{
		mAutoCompleteLines.clear();

		// Nothing to check
		if ( mCmdLine.length() == 0 )
		{
			mHeight = mOriginalHeight;
			return;
		}

		// See if a valid command has been typed
		int firstSpace = mCmdLine.find_first_of( ' ' );
		std::string cmd = mCmdLine.substr( 0, firstSpace );
		StringUtil::StringToLower( cmd );
		auto cmdFound = mCommands.find( cmd );
		if ( cmdFound != mCommands.end() )
		{
			AutoCompleteLine acl;
			acl.line = cmdFound->second->FnName + ' '  + cmdFound->second->FnArgsInfo;
			acl.matchyness = acl.line.length();

			mAutoCompleteLines.push_back( acl );

			AutoCompleteLine acl_help;
			acl_help.line = cmdFound->second->FnHelp;
			acl_help.matchyness = acl_help.line.length();

			if ( acl_help.matchyness > 0 )
			{
				mAutoCompleteLines.push_back( acl_help );
			}

			mHeight = mOriginalHeight + mFont->GetFontHeight() *
				Mathi::Min( (int) mAutoCompleteLines.size(), mMaxAutoCompleteLines );

			return;
		}

		// Check for possible valid commands
		for ( auto itr = mCommands.begin(); itr != mCommands.end(); ++ itr )
		{
			if ( mCmdLine.length() > itr->first.length() ) continue;

			size_t j;
			for ( j = 0; j < mCmdLine.length(); ++j )
			{
				if ( tolower( mCmdLine[j] ) != tolower( itr->first[j] ) ) break;
			}

			if ( j == mCmdLine.length() )
			{
				AutoCompleteLine acl;
				acl.matchyness = j;
				acl.line = itr->first;

				mAutoCompleteLines.push_back( acl );

				if ( mCmdLine == acl.line )
				{
					mAutoCompleteLines.clear();
					mAutoCompleteLines.push_back( acl );
					break;
				}
			}
		}

		mHeight = mOriginalHeight + mFont->GetFontHeight() *
			Mathi::Min( (int) mAutoCompleteLines.size(), mMaxAutoCompleteLines );

		std::sort( mAutoCompleteLines.begin(), mAutoCompleteLines.end() );
	}
	//---------------------------------------


	//---------------------------------------
	// Commands
	//---------------------------------------
	void CommandLine::Clear( const std::string& args )
	{
		ConsoleClear();
	}
	//---------------------------------------
	void CommandLine::Help( const std::string& args )
	{
		for ( auto itr = mCommands.begin(); itr != mCommands.end(); ++itr )
		{
			ConsolePrintf( CONSOLE_INFO, "%-20s%s", itr->second->FnName.c_str(), itr->second->FnHelp.c_str() );
		}
	}
	//---------------------------------------
	void CommandLine::Exit( const std::string& args )
	{
		exit( 0 );
	}
	//---------------------------------------
	void CommandLine::CmdFireEvent( const std::string& args )
	{
		std::vector< std::string > tokens;
		StringUtil::Tokenize( args, tokens );

		if ( tokens.empty() )
		{
			ConsolePrintf( CONSOLE_INFO, ">Usage: FireEvent eventName [params]" );
			return;
		}

		std::string eventName = tokens[0];
		Dictionary params;

		params.Set( "cmdargs", args );

		if ( !EventManager::FireEvent( eventName, params ) )
		{
			ConsolePrintf( C_FG_LIGHT_PURPLE, ">No objects registered for event '%s'", eventName.c_str() );
		}
	}
	//---------------------------------------
	void CommandLine::OnDraw( SDL_Surface* screen ) const
	{
		if ( !mVisible ) return;

		int y = (int) ( mY + GetBaseHeight() );
		int x = 0;
		int cy = y-2;
		y -= mFont->GetFontHeight();
		int cx = (int) GetCursorPos() + mFont->GetTextWidth( ">" );

		// Background
		Surface::DrawRect( screen, x, y, (int) mWidth,   (int) mHeight, Color::BLACK.bgra );
		Surface::DrawRect( screen, x, y, (int) mWidth-1, (int) mHeight, Color::GREEN.bgra, false );

		// Text
		Surface::DrawTextFormat( screen, x, y, mFont, Color::GREEN.bgra, ">%s", GetText().c_str() );
		Surface::DrawLine( screen, cx, cy, cx + mFont->GetTextWidth( "_" ), cy, Color::GREEN.bgra );

		// Render auto-complete box
		const std::vector< CommandLine::AutoCompleteLine >& acls = GetAutoCompleteLines();
		int end = Mathi::Min( (int) acls.size(), GetMaxAutoCompleteLines() );
		for ( int i = 0; i < end; ++i )
		{
			std::string s1 = acls[i].line.substr( 0, acls[i].matchyness );
			std::string s2 = acls[i].line.substr( acls[i].matchyness );

			Surface::DrawText( screen, x, y + ( i+1) * mFont->GetFontHeight(), mFont, Color::GREEN.bgra, s1.c_str() );
			Surface::DrawText( screen, x + mFont->GetTextWidth( s1.c_str() ), y + ( i+1) * mFont->GetFontHeight(), mFont, Color::DARK_GREEN.bgra, s2.c_str() );
		}
	}
	//---------------------------------------
}

//---------------------------------------
bool ConsoleInit()
{
	if ( gConsole->Initialize() )
	{
 		gCommandLine = new CommandLine( gConsole->mWidth, (float) ( gConsole->mConsoleFont->GetFontHeight() + 2 ) );
		gCommandLine->Initialize();
		gCommandLine->SetPositionY( gConsole->mHeight );
		return true;
	}
	return false;
}
//---------------------------------------
void ConsoleCreate( float width, float height )
{
	gConsole = new Console( 2048, width, height );
}
//---------------------------------------
void SetConsoleStyle( mage::uint16 style )
{
	uint16 fgColor = style & 0x0F;
	switch ( fgColor )
	{
	case C_FG_BLACK			 : gConsole->mTextColor = Color::BLACK; break;
	case C_FG_BLUE           : gConsole->mTextColor = Color::BLUE; break;
	case C_FG_GREEN          : gConsole->mTextColor = Color::DARK_GREEN; break;
	case C_FG_AQUA           : gConsole->mTextColor = Color::DARK_BLUE; break;
	case C_FG_RED            : gConsole->mTextColor = Color::DARK_RED; break;
	case C_FG_PURPLE         : gConsole->mTextColor = Color::PURPLE; break;
	case C_FG_YELLOW         : gConsole->mTextColor = Color::DARK_YELLOW; break;
	case C_FG_WHITE          : gConsole->mTextColor = Color::LIGHT_GREY; break;
	case C_FG_GRAY           : gConsole->mTextColor = Color::GREY; break;
	case C_FG_LIGHT_BLUE     : gConsole->mTextColor = Color::LIGHT_BLUE; break;
	case C_FG_LIGHT_GREEN    : gConsole->mTextColor = Color::GREEN; break;
	case C_FG_LIGHT_AQUA     : gConsole->mTextColor = Color::CYAN; break;
	case C_FG_LIGHT_RED      : gConsole->mTextColor = Color::RED; break;
	case C_FG_LIGHT_PURPLE   : gConsole->mTextColor = Color::PINK; break;
	case C_FG_LIGHT_YELLOW   : gConsole->mTextColor = Color::YELLOW; break;
	case C_FG_LIGHT_WHITE    : gConsole->mTextColor = Color::WHITE; break;
	}
}
//---------------------------------------
void ConsolePrintf( mage::uint16 style, const char* fmt, ... )
{
	CriticalBlock( gConsoleMutex );

	SetConsoleStyle( style == CONSOLE_DEFAULT ? DEFAULT_STYLE : style );
	va_list vargs;
	va_start( vargs, fmt );
	gConsole->VPrintf( gConsole->mTextColor, fmt, vargs );
	va_end( vargs );
	SetConsoleStyle( DEFAULT_STYLE );
}
//---------------------------------------
void ConsolePrintf( const char* fmt, ... )
{
	CriticalBlock( gConsoleMutex );

	va_list vargs;
	va_start( vargs, fmt );
	gConsole->VPrintf( gConsole->mTextColor, fmt, vargs );
	va_end( vargs );
}
//---------------------------------------
void ConsoleClear()
{
	gConsole->Clear();
}
//---------------------------------------
void ConsoleRender( void* screen )
{
	gConsole->OnDraw( (SDL_Surface*) screen );
	gCommandLine->OnDraw( (SDL_Surface*) screen );
}
//---------------------------------------
void ConsoleScroll( int dir )
{
	gConsole->Scroll( -dir );
}
//---------------------------------------
void ConsoleToggleVisibility()
{
	gConsole->ToggleVisibility();
}
//---------------------------------------
bool IsConsoleVisible()
{
	return gConsole->IsVisible();
}
//---------------------------------------
bool ConsoleInput( int code, int mod )
{
	return gCommandLine->OnKeyJustDown( code, mod );// && gCommandLine->OnKeyDown( code );
}
//---------------------------------------
void* GetGameConsole()
{
	return gCommandLine;
}
//---------------------------------------
#else
//---------------------------------------
void ConsolePrintf( uint16 style, const char* fmt, ... )
{
	CriticalBlock( gConsoleMutex );

	SetConsoleStyle( style == CONSOLE_DEFAULT ? DEFAULT_STYLE : style );
	va_list vargs;
	va_start( vargs, fmt );
	vprintf( fmt, vargs );
	va_end( vargs );
	SetConsoleStyle( DEFAULT_STYLE );
}
//---------------------------------------
void ConsolePrintf( const char* fmt, ... )
{
	CriticalBlock( gConsoleMutex );

	va_list vargs;
	va_start( vargs, fmt );
	vprintf( fmt, vargs );
	va_end( vargs );
}
//---------------------------------------
void ConsoleClear()
{
	system( "cls" );
}
//---------------------------------------
void ConsoleRender( void* screen )
{}
//---------------------------------------
void ConsoleScroll( int dir )
{}
//---------------------------------------
void ConsoleToggleVisibility()
{}
//---------------------------------------
bool IsConsoleVisible()
{
	return false;
}
//---------------------------------------
bool ConsoleInput( int code, int mod )
{
	return false;
}
//---------------------------------------
bool ConsoleInit()
{
	return true;
}
//---------------------------------------
void ConsoleCreate( float width, float height )
{}
//---------------------------------------
void* GetGameConsole()
{
	return NULL;
}
//---------------------------------------

#endif

//---------------------------------------
void SetConsoleDefaultStyle( uint16 style )
{
	DEFAULT_STYLE = style;
	SetConsoleStyle( DEFAULT_STYLE );
}
//---------------------------------------