/*
 * Author      : Matthew Johnson
 * Date        : 4/Jun/2013
 * Description :
 *   
 */
 
#pragma once

#include "GameScope.h"

#include <map>

namespace mage
{

	class GameEnvironment
	{
	public:
		GameEnvironment();
		~GameEnvironment();



	private:
		GameScope* mGlobalScope;
		std::map< std::string, GameScope* > mScopeMap;
		
	};

}