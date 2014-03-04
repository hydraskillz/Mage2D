/*
 * Author      : Matthew Johnson
 * Date        : 29/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameString
	{
	public:
		GameString();
		~GameString();

		std::string Text;
		Vec2i Position;
		Color TextColor;
		Font* TextFont;
		bool WorldSpace;
	};

}