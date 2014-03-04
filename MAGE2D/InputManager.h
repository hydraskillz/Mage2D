/*
 * Author      : Matthew Johnson
 * Date        : 8/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		bool LoadKeyCodes( const char* filename );
		bool LoadInputBindings( const char* filename );

		void SetKeyDown( int code );
		void SetKeyUp( int code );

		void OnUpdate();

	private:
		enum InputState
		{
			IS_UNKNOWN   = 0x00000000,
			IS_DOWN      = 0x00000001,
			IS_UP        = 0x00000002,
			IS_JUST_DOWN = 0x00000004,
			IS_JUST_UP   = 0x00000008
		};

		struct Key
		{
			std::string Name;
			InputState State;
		};

		struct KeyBinding
		{
			std::string EventName;
			unsigned ModifierMask;
			unsigned StateTriggerMask;
		};

		// <KeyCode, KeyName>
		std::map< int, Key > mKeyMappings;
		// <KeyName, KeyBinding>
		std::multimap< std::string, KeyBinding > mKeyBindings;
	};

}