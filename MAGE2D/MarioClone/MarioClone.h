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

	class MarioClone
		: public MageGame
	{

		MAGE_DECLARE_INIT;
		MAGE_DECLARE_TERM;

	public:
		MarioClone();
		virtual ~MarioClone();

		virtual bool OnInitialize();
	};

	MAGE_REGISTER_INIT( MarioClone );
	MAGE_REGISTER_TERM( MarioClone );
}