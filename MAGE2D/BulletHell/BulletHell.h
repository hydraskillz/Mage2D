/*
 * Author      : Matthew Johnson
 * Date        : 1/Jun/2013
 * Description :
 *   
 */
 
#pragma once

#include <MageGame.h>

namespace mage
{

	class BulletHell
		: public MageGame
	{

		MAGE_DECLARE_INIT;
		MAGE_DECLARE_TERM;

	public:
		BulletHell();
		virtual ~BulletHell();

		virtual bool OnInitialize();
	};

	MAGE_REGISTER_INIT( BulletHell );
	MAGE_REGISTER_TERM( BulletHell );
}