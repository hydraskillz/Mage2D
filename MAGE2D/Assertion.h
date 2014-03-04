/*
 * Author      : Matthew Johnson
 * Date        : 2/Jan/2013
 * Description :
 *   Custom Windows debug assertion.
 */
 
#pragma once

// This define enables/disables the custom assertion class.
// When disabled the default assert() is called.
#define MAGE_USE_ASSERTION

#ifdef MAGE_USE_ASSERTION
namespace mage
{

	class Assertion
	{
	public:
		Assertion( bool condition, const char* file, int line, const char* format, ... );
		~Assertion();

	private:
		static const int MAX_MSG_SIZE = 1024;
		static const char* msDebugPrompt;
		static const size_t msDebugPromptLength;
		static const char* msMessagePrefix;
		static const char* msMessageBoxTitle;
	};

}
#	define assertion( condition, format, ... )											\
		mage::Assertion( condition, __FILE__, __LINE__, format, __VA_ARGS__ )
#ifdef _DEBUG
#	define DebugAsssertion( condition, format, ... )									\
		mage::Assertion( condition, __FILE__, __LINE__, format, __VA_ARGS__ )
#else
#	define DebugAsssertion( condition, format, ... )
#endif
#else
#	define assertion( condition, format, ... ) assert( condition )
#endif