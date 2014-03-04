/*
 * Author      : Matthew Johnson
 * Date        : 6/Jun/2013
 * Description :
 *   Welcome to macro hell.
 */
 
#pragma once

#include "MageLib.h"

#define DEFINITION_GETBYNAME( DEF_CLASS_NAME, T_CLASS_NAME  )						\
	static DEF_CLASS_NAME* GetDefinitionByName( const std::string& name )			\
	{																				\
		auto found = T_CLASS_NAME##Definitions.find( name );						\
		if ( found == T_CLASS_NAME##Definitions.end() )								\
		{																			\
			return NULL;															\
		}																			\
		return found->second;														\
	}

#define DEFINITION_DESTROYALL( T_CLASS_NAME )										\
	static void DestroyAllDefinitions()												\
	{																				\
		DestroyMapByValue( T_CLASS_NAME##Definitions )								\
	}

#define DEFINITION_REGISTERDEFINITION( DEF_CLASS_NAME, T_CLASS_NAME )				\
	void RegisterDefinition( const std::string& name )								\
	{																				\
		DEF_CLASS_NAME*& existingDefinition = T_CLASS_NAME##Definitions[ name ];	\
		if ( existingDefinition )													\
		{																			\
			delete existingDefinition;												\
		}																			\
		existingDefinition = this;													\
	}

// Creates the static functions for Definition classes.
// The name-to-definition map is named T_CLASS_NAMEDefinitions i.e. EntityDefinitions
// DEF_CLASS_NAME  : name of the Definition class
// T_CLASS_NAME    : name of the type of class the Definition creates
#define DEFINE_DEFINITION_BASE_MEMBERS( DEF_CLASS_NAME, T_CLASS_NAME )				\
	public:																			\
		DEFINITION_GETBYNAME( DEF_CLASS_NAME, T_CLASS_NAME )						\
		DEFINITION_DESTROYALL( T_CLASS_NAME )										\
																					\
	protected:																		\
		DEFINITION_REGISTERDEFINITION( DEF_CLASS_NAME, T_CLASS_NAME )				\
																					\
	private:																		\
		static std::map< std::string, DEF_CLASS_NAME* > T_CLASS_NAME##Definitions;
	
// Put this in the .cpp to declare static the members
#define DECLARE_DEFINITION_BASE_MEMBERS( DEF_CLASS_NAME, T_CLASS_NAME ) \
	std::map< std::string, DEF_CLASS_NAME* > DEF_CLASS_NAME::T_CLASS_NAME##Definitions;