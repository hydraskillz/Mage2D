#include "MageLib.h"

using namespace mage;

//---------------------------------------
InputManager::InputManager()
{}
//---------------------------------------
InputManager::~InputManager()
{}
//---------------------------------------
bool InputManager::LoadKeyCodes( const char* filename )
{
	//if ( !FileExists( filename ) )
	//{
	//	ConsolePrintf( CONSOLE_WARNING, "Warning: KeyCode file '%s' not found. Input will not work.\n", filename );
	//	return false;
	//}

	// Clear old mapping
	mKeyMappings.clear();

	XmlReader reader( filename );
	XmlReader::XmlReaderIterator rootItr = reader.ReadRoot();
	if ( !rootItr.IsValid() )
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: KeyCode file '%s' not found. Input will not work.\n", filename );
		return false;
	}

	rootItr.ValidateXMLAttributes( "", "" );
	rootItr.ValidateXMLChildElemnts( "", "Key" );

	for ( XmlReader::XmlReaderIterator keyItr = rootItr.NextChild( "Key" );
		  keyItr.IsValid(); keyItr = keyItr.NextSibling( "Key" ) )
	{
		keyItr.ValidateXMLAttributes( "name,value", "" );

		std::string name = keyItr.GetAttributeAsString( "name" );
		int value        = keyItr.GetAttributeAsInt( "value" );

		Key key = { name, IS_UP };

		// Insert mapping into map, showing warning if mapping exists already
		std::pair< std::map< int, Key >::iterator, bool > insert =
			mKeyMappings.insert( std::make_pair( value, key ) );
		if ( !insert.second )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: KeyCode %d : '%s' already defined!\n", value, name.c_str() );
		}
	}

	return true;
}
//---------------------------------------
bool InputManager::LoadInputBindings( const char* filename )
{
	static bool _init_tables = true;
	static std::map< std::string, int > StateTagToEnumMap;

	if ( _init_tables )
	{
		_init_tables = false;

		StateTagToEnumMap[ "UP"        ] = IS_UP;
		StateTagToEnumMap[ "DOWN"      ] = IS_DOWN;
		StateTagToEnumMap[ "JUST_UP"   ] = IS_JUST_UP;
		StateTagToEnumMap[ "JUST_DOWN" ] = IS_JUST_DOWN;
	}

	//if ( !FileExists( filename ) )
	//{
	//	ConsolePrintf( CONSOLE_WARNING, "Warning: InputBinding file '%s' not found. Input will not work.\n", filename );
	//	return false;
	//}

	// Clear old mapping
	mKeyBindings.clear();

	XmlReader reader( filename );
	XmlReader::XmlReaderIterator rootItr = reader.ReadRoot();
	if ( !rootItr.IsValid() )
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: InputBinding file '%s' not found. Input will not work.\n", filename );
		return false;
	}

	rootItr.ValidateXMLAttributes( "", "" );
	rootItr.ValidateXMLChildElemnts( "", "Binding" );

	for ( XmlReader::XmlReaderIterator bindingItr = rootItr.NextChild( "Binding" );
		bindingItr.IsValid(); bindingItr = bindingItr.NextSibling( "Binding" ) )
	{
		bindingItr.ValidateXMLAttributes( "name,key,state", "modifiers" );

		std::string name      = bindingItr.GetAttributeAsString( "name" );
		std::string modifiers = bindingItr.GetAttributeAsString( "modifiers", "" );
		std::string key       = bindingItr.GetAttributeAsString( "key" );
		std::string state     = bindingItr.GetAttributeAsString( "state" );

		KeyBinding keyBinding = { name, 0, 0 };

		std::vector< std::string > states;
		StringUtil::Tokenize( state, states, "|" );
		for ( auto itr = states.begin(); itr != states.end(); ++itr )
		{
			std::string stateName = StringUtil::Strip( *itr );
			keyBinding.StateTriggerMask |= GetMappedValue( StateTagToEnumMap, stateName, "Unknown keystate value." );
		}

		mKeyBindings.insert( std::make_pair( key, keyBinding ) );
	}

	return true;
}
//---------------------------------------
void InputManager::SetKeyDown( int code )
{
	Key& key = mKeyMappings[ code ];

	if ( key.State == IS_UP )
	{
		key.State = IS_JUST_DOWN;
	}
	else
	{
		key.State = IS_DOWN;
	}
}
//---------------------------------------
void InputManager::SetKeyUp( int code )
{
	Key& key = mKeyMappings[ code ];

	if ( key.State == IS_DOWN )
	{
		key.State = IS_JUST_UP;
	}
	else
	{
		key.State = IS_UP;
	}
}
//---------------------------------------
void InputManager::OnUpdate()
{
	for ( auto keyItr = mKeyMappings.begin(); keyItr != mKeyMappings.end(); ++keyItr )
	{
		auto binding = mKeyBindings.equal_range( keyItr->second.Name );
		if ( binding.first != binding.second )
		{
			for ( auto bindingItr = binding.first; bindingItr != binding.second; ++bindingItr )
			{
				if ( bindingItr->second.StateTriggerMask & keyItr->second.State )
				{
					Dictionary params;
					params.Set( "EventName", bindingItr->second.EventName );
					EventManager::FireEvent( bindingItr->second.EventName, params );
				}
			}
		}

		if ( keyItr->second.State == IS_JUST_DOWN )
		{
			keyItr->second.State = IS_DOWN;
		}
		else if ( keyItr->second.State == IS_JUST_UP )
		{
			keyItr->second.State = IS_UP;
		}
	}
}
//---------------------------------------