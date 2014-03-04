/*
 * Author      : Matthew Johnson
 * Date        : 16/Jul/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class PythonWrapper
	{
	public:
		static bool Initialize();
		static void ShutDown();
		static void RunScript( const char* script_name );
		static void RunScriptOn( const char* script_name, GameObject* owner );
		//static void ExeFunction( const char* py_source, const char* py_function );

	};

}