#include "MageLib.h"

using namespace mage;

extern "C"
{

//---------------------------------------
// Helper for getting mapped value
int GetMappedValue(
	const std::map< std::string, int >& map,
	const std::string& tag,
	const char* errorMsg,
	int defaultValue )
{
	auto itr = map.find( tag );
	if ( itr != map.end() )
	{
		return itr->second;
	}
	ConsolePrintf( CONSOLE_WARNING, "Warning: '%s' : %s\n", tag.c_str(), errorMsg );
	return defaultValue;
}
//---------------------------------------
bool GetPropertiesFromXML( const XmlReader::XmlReaderIterator& properitesItr, Dictionary& properties )
{
	if ( !properitesItr.IsValid() )
	{
		ConsolePrintf( CONSOLE_ERROR, "Invalid property itr.\n" );
		return false;
	}

	if ( !properitesItr.ElementNameEquals( "Properties" ) )
	{
		ConsolePrintf( CONSOLE_ERROR, "Itr not Properties node.\n" );
		return false;
	}

	// Dump property tags into Dictionary
	for ( XmlReader::XmlReaderIterator  propItr = properitesItr.NextChild( "Property" );
		propItr.IsValid(); propItr = propItr.NextSibling( "Property" ) )
	{
		propItr.ValidateXMLAttributes( "name,value","type" );

		std::string name = propItr.GetAttributeAsString( "name" );
		std::string type = propItr.GetAttributeAsString( "type", "String" );

		if ( !type.compare( "String" ) )
		{
			std::string value = propItr.GetAttributeAsString( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Int" ) )
		{
			int value = propItr.GetAttributeAsInt( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Unsigned Int" ) )
		{
			unsigned value = propItr.GetAttributeAsUInt( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Float" ) )
		{
			float value = propItr.GetAttributeAsFloat( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Vec2" ) )
		{
			Vec2f value = propItr.GetAttributeAsVec2f( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Vec3" ) )
		{
			Vec3f value = propItr.GetAttributeAsVec3f( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Vec4" ) )
		{
			Vec4f value = propItr.GetAttributeAsVec4f( "value" );
			properties.Set( name, value );
		}
		else if ( !type.compare( "Color" ) )
		{
			Color value = propItr.GetAttributeAsColor( "value" );
			properties.Set( name, value );
		}
	}

	return true;
}
//---------------------------------------
}