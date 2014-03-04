/*
 * Author      : Matthew Johnson
 * Date        : 19/Apr/2013
 * Description :
 *   Multi-type container keyed on strings.
 */
 
#pragma once

namespace mage
{

	//---------------------------------------
	// DictionaryValueBase
	// Base for values in Dictionary
	//---------------------------------------
	class DictionaryValueBase
	{
	public:
		virtual ~DictionaryValueBase() {}

		virtual DictionaryValueBase* Copy() = 0;

	protected:
		DictionaryValueBase() {}
	};
	//---------------------------------------


	//---------------------------------------
	// DictionaryValue
	// Values stored in Dictionary
	//---------------------------------------
	template< typename T >
	class DictionaryValue
		: public DictionaryValueBase
	{
	public:
		DictionaryValue( const T& value )
			: mValue( value )
		{}
		virtual ~DictionaryValue() {}

		virtual DictionaryValueBase* Copy()
		{
			return new DictionaryValue< T >( mValue );
		}

		const T& GetValue() const
		{
			return mValue;
		}

	private:
		T mValue;
	};
	//---------------------------------------


	//---------------------------------------
	// Dictionary
	// A multi-type container that stores values by string keys
	//---------------------------------------
	class Dictionary
	{
	public:
		enum DictionaryError
		{
			DErr_SUCCESS = 0,
			DErr_NO_ENTRY,
			DErr_WRONG_TYPE,
			DErr_EMPTY,
			DErr_COUNT
		};

		static const char* GetErrorString( DictionaryError error )
		{
			static char* errorStrings[] = 
			{
				"Success",
				"No Entry",
				"Wrong Type",
				"Empty Container",
			};

			// Bad error code
			if ( error < 0 || error >= DErr_COUNT )
			{
				return "Invalid ErrorCode";
			}

			return errorStrings[ error ];
		}

		Dictionary();
		Dictionary( const Dictionary& other );
		virtual ~Dictionary();

		Dictionary& operator=( const Dictionary& other )
		{
			for ( auto itr = other.mDictionary.begin();
				  itr != other.mDictionary.end(); ++itr )
			{
				mDictionary[ itr->first ] = itr->second->Copy();
			}

			return *this;
		}

		template< typename T >
		void Set( const std::string& key, const T& value )
		{
			// Delete the value if it already exists and set the new value
			DictionaryValueBase*& v = mDictionary[ key ];
			Delete0( v );
			v = new DictionaryValue< T >( value );
		}

		// Get a named property from the Dictionary of a given type
		// Returns DErr_SUCCESS on success or an error code on failure
		// value is not modified on failure
		template< typename T >
		DictionaryError Get( const std::string& key, T& value ) const
		{
			// Error on empty
			if ( mDictionary.empty() )
			{
				return DErr_EMPTY;
			}

			// Search for key and return error if not found
			auto found = mDictionary.find( key );
			if ( found == mDictionary.end() )
			{
				return DErr_NO_ENTRY;
			}

			// Try to cast the found value to the type of value
			DictionaryValue< T >* v = dynamic_cast< DictionaryValue< T >* >( found->second );

			// Error if wrong type
			if ( !v )
			{
				return DErr_WRONG_TYPE;
			}

			// Everything went ok - set the output value and return success
			value = v->GetValue();
			return DErr_SUCCESS;
		}

	private:
		std::map< std::string, DictionaryValueBase* > mDictionary;
	};
	//---------------------------------------

}