/*
 * Author      : Matthew Johnson
 * Date        : 4/Jun/2013
 * Description :
 *   
 */
 
#pragma once

#include "GameVar.h"

#include <map>

namespace mage
{

	class GameScope
	{
	public:
		GameScope( const char* name, GameScope* parent=NULL );
		~GameScope();
	
		void SetParentScope( GameScope* scope );
		void SetChildScope( GameScope* scope );
		void AddGameVar( GameVar& var );
		GameVar& GetGameVar() const;
		
	private:
		const char* mName;
		GameScope* mParentScope;
		GameScope* mChildScope;

		std::map< std::string, GameVar > mGameVars;
	};

}