#include "MageLib.h"

using namespace mage;

GameInfo* GameInfo::msGameInfo;

//---------------------------------------
GameInfo::GameInfo()
{}
//---------------------------------------
GameInfo::~GameInfo()
{}
//---------------------------------------
void GameInfo::CreateGameInfo( const char* xmlDoc )
{
	DestroyGameInfo();
	msGameInfo = new GameInfo();

	XmlReader reader( xmlDoc );
	XmlReader::XmlReaderIterator root = reader.ReadRoot();

	// Fall back to defaults if not GameInfo exists
	if ( !root.IsValid() )
	{
		ConsolePrintf( CONSOLE_WARNING, "GameInfo '%s' not found. Using default GameInfo.\n", xmlDoc );
		//msGameInfo->GameName = "MAGE Game";
		msGameInfo->StartUpMap = "";
		return;
	}

	root.ValidateXMLAttributes( "","gameName,startUpMap" );
	root.ValidateXMLChildElemnts( "","Properties" );

	msGameInfo->GameName = root.GetAttributeAsString( "gameName" );
	msGameInfo->StartUpMap = root.GetAttributeAsString( "startUpMap" );

	// Current level is initial level
	msGameInfo->CurrentLevel = msGameInfo->StartUpMap;
	// Initial level is loaded by default on load
	msGameInfo->ShouldLoadLevel = false;

	XmlReader::XmlReaderIterator properties = root.NextChild( "Properties" );
	//GetPropertiesFromXML( properties, msGameInfo->Properties );
	if ( properties.IsValid() )
	{
		msGameInfo->LoadProperties( properties );
	}
}
//---------------------------------------
void GameInfo::DestroyGameInfo()
{
	Delete0( msGameInfo );
}
//---------------------------------------
GameInfo& GameInfo::GetGameInfo()
{
	return *msGameInfo;
}
//---------------------------------------
void GameInfo::LoadProperties( const XmlReader::XmlReaderIterator& propsItr )
{
	// @TODO move this to a static function in GameVar
	static bool _init_tables = false;
	static std::map< std::string, int > GameVarTypeMap;
	if ( !_init_tables )
	{
		_init_tables = true;

		GameVarTypeMap[ "null"    ] = GV_NULL;
		GameVarTypeMap[ "int"     ] = GV_INT;
		GameVarTypeMap[ "float"   ] = GV_FLOAT;
		GameVarTypeMap[ "string"  ] = GV_STRING;
	}

	for ( XmlReader::XmlReaderIterator propItr = propsItr.NextChild( "Property" );
		propItr.IsValid(); propItr = propItr.NextSibling( "Property" ) )
	{
		propItr.ValidateXMLAttributes( "name,type,value","debug,displayColor" );

		std::string name   = propItr.GetAttributeAsString( "name" );
		std::string type   = propItr.GetAttributeAsString( "type" );
		GameVarType gvType = (GameVarType) GetMappedValue( GameVarTypeMap, type, "Unknown GameVar type", GV_NULL );
		Color       color  = propItr.GetAttributeAsColor( "displayColor", Color::WHITE );

		name = "GameInfo." + name;

		GameVar*& gvar = Properties[ name ];

		if ( !gvar )
		{
			if ( gvType == GV_INT )
			{
				int data = propItr.GetAttributeAsInt( "value" );
				gvar = new GameVar( name.c_str(), gvType, (void*)&data );
			}
			else if ( gvType == GV_FLOAT )
			{
				float data = propItr.GetAttributeAsFloat( "value" );
				gvar = new GameVar( name.c_str(), gvType, (void*)&data );
			}
			else if ( gvType == GV_STRING )
			{
				std::string data = propItr.GetAttributeAsString( "value" );
				gvar = new GameVar( name.c_str(), gvType, (void*)&data );
			}

			gvar->SetDisplayColor( color );

			//ConsolePrintf( C_FG_LIGHT_PURPLE, "%s: GameVar loaded:\n %s\n"
			//	, mName.c_str()
			//	, gvar->ToString().c_str() );
		}
		else
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: '%s' already defined. Duplicate not loaded. Please make sure your Property names are unique per GameObject.\n"
				, name.c_str() );
			continue;
		}
	}
}
//---------------------------------------