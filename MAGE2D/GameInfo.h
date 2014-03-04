/*
 * Author      : Matthew Johnson
 * Date        : 3/Jun/2013
 * Description :
 *   Describes a game.
 */
 
#pragma once

namespace mage
{

	class GameInfo
	{
		GameInfo();
	public:
		~GameInfo();

		// Create a new GameInfo object, destroying the previous (if one existed)
		static void CreateGameInfo( const char* xmlDoc );
		// Destroy the GameInfo object
		static void DestroyGameInfo();
		// Get the current GameInfo object
		static GameInfo& GetGameInfo();

		std::string GameName;
		std::string StartUpMap;
		std::string CurrentLevel;
		bool ShouldLoadLevel;
		std::map< std::string, GameVar* >  Properties;

	private:
		void LoadProperties( const XmlReader::XmlReaderIterator& propsItr );

		static GameInfo* msGameInfo;
	};

}