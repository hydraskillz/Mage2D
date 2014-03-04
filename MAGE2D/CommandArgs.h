/*
 * Author      : Matthew Johnson
 * Date        : 12/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class CommandArgs
	{
	public:
		CommandArgs( int argc, char** argv );
		~CommandArgs();

		template< typename T >
		bool GetArgAs( const char* param, T& value, unsigned int index=0 ) const;

		bool HasParam( const char* param ) const;

	private:
		// < Param, [ Args ] >
		mutable std::map< std::string, std::vector< std::string > > mCommandMap;

	};

	template< typename T >
	bool CommandArgs::GetArgAs( const char* param, T& value, unsigned int index ) const
	{
		if ( !HasParam( param ) )
		{
			return false;
		}

		std::vector< std::string >& args = mCommandMap[ param ];

		if ( index < 0 || index >= args.size() )
		{
			return false;
		}

		return StringUtil::StringToType( args[ index ], &value );
	}

}