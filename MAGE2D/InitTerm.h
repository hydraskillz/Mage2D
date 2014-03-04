/*
 * Author      : Matthew Johnson
 * Date        : 16/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class InitTerm
	{
	public:
		typedef void (*Initializer)( void );
		static void AddInitializer( Initializer function );
		static void RunInitializers();

		typedef void (*Terminator) ( void );
		static void AddTerminator( Terminator function );
		static void RunTerminators();

	private:
		// Increase this as needed
		// Using static array size to avoid dynamic allocation
		enum { MAX_INITTERM = 512 };

		static int msInitializerCount;
		static Initializer msIntializers[ MAX_INITTERM ];

		static int msTerminatorCount;
		static Terminator msTerminators[ MAX_INITTERM ];

	};

//---------------------------------------
// Macros
//---------------------------------------
#define MAGE_DECLARE_INIT \
public: \
	static bool RegisterInitializer(); \
	static void Initialize(); \
private: \
	static bool msInitializerRegistered;
//---------------------------------------
#define MAGE_IMP_INIT( classname ) \
	bool classname::msInitializerRegistered = false; \
	bool classname::RegisterInitializer() \
	{ \
		if ( !msInitializerRegistered ) \
		{ \
			InitTerm::AddInitializer( classname::Initialize ); \
			msInitializerRegistered = true; \
		} \
		return msInitializerRegistered; \
	}
//---------------------------------------
#define MAGE_REGISTER_INIT( classname ) \
	static bool gsInitializerRegistered_##classname = \
		classname::RegisterInitializer();
//---------------------------------------
#define MAGE_DECLARE_TERM \
public: \
	static bool RegisterTerminator(); \
	static void Terminate(); \
private: \
	static bool msTerminatorRegistered;
//---------------------------------------
#define MAGE_IMP_TERM( classname ) \
	bool classname::msTerminatorRegistered = false; \
	bool classname::RegisterTerminator() \
	{ \
		if (!msTerminatorRegistered ) \
		{ \
			InitTerm::AddTerminator( classname::Terminate ); \
			msTerminatorRegistered = true; \
		} \
		return msTerminatorRegistered; \
	}
//---------------------------------------
#define MAGE_REGISTER_TERM( classname ) \
	static bool gsTerminatorRegistered_##classname = \
		classname::RegisterTerminator();
//---------------------------------------
}