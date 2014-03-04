#include "MageLib.h"

using namespace mage;

//---------------------------------------
// String Utils
//---------------------------------------
std::string StringUtil::StringToLower( const std::string& str )
{
	std::string _ret = str;
	StringToLower( _ret );
	return _ret;
}
//---------------------------------------
void StringUtil::StringToLower( std::string& str )
{
	std::transform( str.begin(), str.end(), str.begin(), ::tolower );
}
//---------------------------------------
std::string StringUtil::StringToUpper( const std::string& str )
{
	std::string _ret = str;
	StringToUpper( _ret );
	return _ret;
}
//---------------------------------------
void StringUtil::StringToUpper( std::string& str )
{
	std::transform( str.begin(), str.end(), str.begin(), ::toupper );
}
//---------------------------------------
void StringUtil::Tokenize( const std::string& str, std::vector< std::string >& tokens, const std::string& pattern )
{
	size_t start = 0;
	size_t end = str.find_first_of( pattern );

	while ( end != std::string::npos )
	{
		tokens.push_back( str.substr( start, end - start ) );
		start = end + 1;
		end = str.find( pattern, start );
	}

	tokens.push_back( str.substr( start, end - start ) );
}
//---------------------------------------
int StringUtil::StringiCmp( const std::string& A, const std::string& B )
{
	return _stricmp( A.data(), B.data() );
}
//---------------------------------------
std::string StringUtil::ExtractFilenameFromPath( const std::string& path, bool keepExtension )
{
	std::string _ret;
	unsigned int lastSlash = path.find_last_of( "/" );
	if ( lastSlash != std::string::npos )
	{
		_ret = path.substr( lastSlash + 1 );
	}
	else
	{
		_ret = path;
	}

	if ( !keepExtension )
	{
		unsigned int lastDot = _ret.find_last_of( "." );
		if ( lastDot != std::string::npos )
		{
			_ret = _ret.substr( 0, lastDot );
		}
	}

	return _ret;
}
//---------------------------------------
std::string StringUtil::StripFilenameFromPath( const std::string& path )
{
	std::string _ret;
	unsigned int lastSlash = path.find_last_of( "/" );
	if ( lastSlash != std::string::npos )
	{
		_ret = path.substr( 0, lastSlash + 1 );
	}
	else
	{
		_ret = path;
	}

	return _ret;
}
//---------------------------------------
std::string StringUtil::LStrip( const std::string& str, const std::string pattern )
{
	std::string _ret = str;

	for ( unsigned int c = 0; c < _ret.length(); ++c )
	{
		std::string sub = _ret.substr( c, pattern.length() );
		if ( sub == pattern )
		{
			_ret = _ret.substr( c + pattern.length() );
		}
		else
		{
			break;
		}
	}

	return _ret;
}
//---------------------------------------
std::string StringUtil::RStrip( const std::string& str, const std::string pattern )
{
	std::string _ret = str;

	for ( int c = _ret.length(); c >= 0; --c )
	{
		if ( c - pattern.length() < 0 ) break;
		std::string sub = _ret.substr( c - pattern.length(), pattern.length() );
		if ( sub == pattern )
		{
			_ret = _ret.substr( 0, c - pattern.length() );
		}
		else
		{
			break;
		}
	}

	return _ret;
}
//---------------------------------------
std::string StringUtil::Strip( const std::string& str, const std::string pattern )
{
	return RStrip( LStrip( str, pattern ), pattern );
}
//---------------------------------------