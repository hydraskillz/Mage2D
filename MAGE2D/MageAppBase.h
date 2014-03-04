/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   
 */
 
#pragma once

#include "MageLib.h"

namespace mage
{

	class MageAppBase
	{
	protected:
		MageAppBase();
	public:
		virtual ~MageAppBase();

		// Application Object
		static MageAppBase* TheGame;

		// Derived classes must set this function pointer
		typedef int (*EntryPoint)( int, char** );
		static EntryPoint Run;

		// Entry point
		virtual int Main( int argc, char** argv ) = 0;
	};

}