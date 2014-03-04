/*
 * Author      : Matthew Johnson
 * Date        : 4/Jun/2013
 * Description :
 *   
 */
 
#pragma once

#include <string>

namespace mage
{

	enum GameVarType
	{
		GV_NULL,
		GV_INT,
		GV_FLOAT,
		GV_STRING
	};

	class GameVar
	{
	public:
		GameVar( const char* name, GameVarType type, void* data );
		GameVar( const char* name, GameVarType type, const std::string& strValue );
		GameVar* Copy() const;
		~GameVar();

		static GameVar NULL_VAR;

		static bool AreSameType( const GameVar& A, const GameVar& B );
		static bool AreCompatableTypes( const GameVar& A, const GameVar& B );
		static void GameVarFromString( GameVar& out_var, const std::string& strVal );

		inline bool IsNullVar() const;
		inline const char* GetName() const;
		inline GameVarType GetType() const;

		// Returns the data as a human readable string
		std::string ToString() const;

		// Returns the value as a string
		std::string ValueString() const;

		// Get the data value as the specified type
		// Returns true if successful
		template< typename T >
		bool GetDataAs( T* type ) const;

		// Does what it sounds like
		void SetDataToInitialValue();

		inline void SetDisplayColor( const Color& color );
		inline const Color& GetDisplayColor() const;
		
		GameVar& operator=( const GameVar& other );
		GameVar& operator=( void* value );

		// Any operation on NULL_VAR throws a NULL exception

		// int + int -> int
		// int + float -> float
		// int + string -> Type Error
		// 
		// float + float -> float
		// float + int -> float
		// float + string -> Type Error
		//
		// string + int -> strcat( string, toString( int ) )
		// string + float -> strcat( string, toString( float ) )
		// string + string -> strcat( string, string )

		GameVar operator+( const GameVar& other ) const;
		GameVar operator-( const GameVar& other ) const;
		GameVar operator*( const GameVar& other ) const;
		GameVar operator/( const GameVar& other ) const;

		GameVar& operator+=( const GameVar& other );
		GameVar& operator-=( const GameVar& other );
		GameVar& operator*=( const GameVar& other );
		GameVar& operator/=( const GameVar& other );

		bool operator==( const GameVar& other ) const;
		bool operator!=( const GameVar& other ) const;
		bool operator< ( const GameVar& other ) const;
		bool operator<=( const GameVar& other ) const;
		bool operator> ( const GameVar& other ) const;
		bool operator>=( const GameVar& other ) const;

	private:
		template< typename T >
		inline void AssignData( void* data, const void* target );
		void AssignData( const void* data );

		template< typename T >
		inline void AddData( void* data, const void* target );
		void AddData( const void* data );

		char* mName;
		GameVarType mType;
		void* mData;
		void* mInitialData;
		Color mDisplayColor;

	};

#include "GameVar.hpp"

}