/*
 * Author      : Matthew Johnson
 * Date        : 4/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	extern "C"
	{

	bool GetPropertiesFromXML( const XmlReader::XmlReaderIterator& propItr, Dictionary& properties );
	int GetMappedValue( const std::map< std::string, int >& map, const std::string& tag, const char* errorMsg, int defaultValue=0 );

	}

}