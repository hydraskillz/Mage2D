#include "MageLib.h"
#include "GameVar.h"

using namespace mage;

GameVar GameVar::NULL_VAR( "NULL", GV_NULL, NULL );

//---------------------------------------
GameVar::GameVar( const char* name, GameVarType type, void* data )
	: mType( type )
	, mDisplayColor( Color::WHITE )
{
	if ( name )
	{
		int len = strlen( name ) + 1;
		mName = new char[ len ];
		strcpy_s( (char*) mName, len, name );
	}
	else
	{
		mName = 0;
	}

	if ( type == GV_INT )
	{
		mData = new int( *(int*)data );
		mInitialData = new int( *(int*)data );
	}
	else if ( type == GV_FLOAT )
	{
		mData = new float( *(float*)data );
		mInitialData = new float( *(float*)data );
	}
	else if ( type == GV_STRING )
	{
		mData = new std::string( *(std::string*)data );
		mInitialData = new std::string( *(std::string*)data );
	}
}
//---------------------------------------
GameVar::GameVar( const char* name, GameVarType type, const std::string& strValue )
	: mType( type )
	, mDisplayColor( Color::WHITE )
{
	if ( name )
	{
		int len = strlen( name ) + 1;
		mName = new char[ len ];
		strcpy_s( (char*) mName, len, name );
	}
	else
	{
		mName = 0;
	}

	if ( type == GV_INT )
	{
		mData = new int;
		mInitialData = new int;
	}
	else if ( type == GV_FLOAT )
	{
		mData = new float;
		mInitialData = new float;
	}
	else if ( type == GV_STRING )
	{
		mData = new std::string;
		mInitialData = new std::string;
	}

	GameVarFromString( *this, strValue );
}
//---------------------------------------
GameVar* GameVar::Copy() const
{
	return new GameVar( mName, mType, mData );
}
//---------------------------------------
GameVar::~GameVar()
{
	if ( mName )
		delete[] mName;
	delete mData;
	delete mInitialData;
}
//---------------------------------------
bool GameVar::AreSameType( const GameVar& A, const GameVar& B )
{
	return A.mType == B.mType;
}
//---------------------------------------
bool GameVar::AreCompatableTypes( const GameVar& A, const GameVar& B )
{
	return A.mType == B.mType;
}
//---------------------------------------
void GameVar::GameVarFromString( GameVar& out_var, const std::string& strVal )
{
	switch ( out_var.GetType() )
	{
		case GV_INT:
		{
			int v;
			if ( StringUtil::StringToType( strVal, &v ) )
			{
				out_var = (void*)&v;
			}
			else
			{
				assertion( false, "Failed to convert string to int.\n" );
			}
			break;
		}
		case GV_FLOAT:
		{
			float v;
			if ( StringUtil::StringToType( strVal, &v ) )
			{
				out_var = (void*)&v;
			}
			else
			{
				assertion( false, "Failed to convert string to float.\n" );
			}
			break;
		}
		case GV_STRING:
		{
			out_var = (void*)&strVal;
			break;
		}
		case GV_NULL:
		default:
			break;
	}
}
//---------------------------------------
std::string GameVar::ToString() const
{
	std::string str( mName );
	str += " = ";
	
#define DATA_TO_STRING( T ) \
	str += StringUtil::ToString( *(T*) mData )

	switch ( mType )
	{
		case GV_INT:
		{
			DATA_TO_STRING( int );
			break;
		}

		case GV_FLOAT:
		{
			DATA_TO_STRING( float );
			break;
		}

		case  GV_STRING:
		{
			str += *(std::string*) mData;
			break;
		}
	}

#undef DATA_TO_STRING

	return str;
}
//---------------------------------------
std::string GameVar::ValueString() const
{
	std::string str;
#define DATA_TO_STRING( T ) \
	str = StringUtil::ToString( *(T*) mData )

	switch ( mType )
	{
	case GV_INT:
		{
			DATA_TO_STRING( int );
			break;
		}

	case GV_FLOAT:
		{
			DATA_TO_STRING( float );
			break;
		}

	case  GV_STRING:
		{
			return *(std::string*) mData;
			break;
		}
	case GV_NULL:
		{
			return "NULL";
			break;
		}
	}

#undef DATA_TO_STRING

	return str;
}
//---------------------------------------
void GameVar::SetDataToInitialValue()
{
	AssignData( mInitialData );
}
//---------------------------------------
void GameVar::AssignData( const void* data )
{
	switch ( mType )
	{
		case GV_INT:
		{
			AssignData< int >( mData, data );
			break;
		}

		case GV_FLOAT:
		{
			AssignData< float >( mData, data );
			break;
		}

		case  GV_STRING:
		{
			AssignData< std::string >( mData, data );
			break;
		}

	default:
		break;
	}
}
//---------------------------------------
void GameVar::AddData( const void* data )
{
	switch ( mType )
	{
		case GV_INT:
		{
			AddData< int >( mData, data );
			break;
		}

		case GV_FLOAT:
		{
			AddData< float >( mData, data );
			break;
		}

		case  GV_STRING:
		{
			AddData< std::string >( mData, data );
			break;
		}

	default:
		break;
	}
}
//---------------------------------------
GameVar& GameVar::operator=( const GameVar& other )
{
	// Simple assignment
	if ( AreSameType( *this, other ) )
	{
		AssignData( other.mData );
	}
	// Implicit cast assignment
	else if ( AreCompatableTypes( *this, other ) )
	{
	}
	// Not assignable types
	else
	{
		return NULL_VAR;
	}

	return *this;
}
//---------------------------------------
GameVar& GameVar::operator=( void* value )
{
	AssignData( value );
	return *this;
}
//---------------------------------------
GameVar GameVar::operator+( const GameVar& other ) const
{
	return NULL_VAR;
}
GameVar GameVar::operator-( const GameVar& other ) const
{
	return NULL_VAR;
}
//---------------------------------------
GameVar GameVar::operator*( const GameVar& other ) const
{
	return NULL_VAR;
}
//---------------------------------------
GameVar GameVar::operator/( const GameVar& other ) const
{
	return NULL_VAR;
}
//---------------------------------------
GameVar& GameVar::operator+=( const GameVar& other )
{
	if ( AreSameType( *this, other ) )
	{
		AddData( other.mData );
	}
	return *this;
}
//---------------------------------------
GameVar& GameVar::operator-=( const GameVar& other )
{
	return *this;
}
//---------------------------------------
GameVar& GameVar::operator*=( const GameVar& other )
{
	return *this;
}
//---------------------------------------
GameVar& GameVar::operator/=( const GameVar& other )
{
	return *this;
}
//---------------------------------------
bool GameVar::operator==( const GameVar& other ) const
{
	if ( AreSameType( other, *this ) )
	{
		switch ( mType )
		{
			case GV_INT:
			{
				return *(int*)other.mData == *(int*)mData;
			}

			case GV_FLOAT:
			{
				return *(float*)other.mData == *(float*)mData;
			}

			case  GV_STRING:
			{
				return *(std::string*)other.mData == *(std::string*)mData;
			}

			case GV_NULL:
			{
				return true;
			}

			default:
				break;
		}
	}
	return false;
}
//---------------------------------------
bool GameVar::operator!=( const GameVar& other ) const
{
	return !( other == *this );
}
//---------------------------------------
bool GameVar::operator< ( const GameVar& other ) const
{
	if ( AreSameType( other, *this ) )
	{
		switch ( mType )
		{
		case GV_INT:
			{
				return *(int*)mData < *(int*)other.mData;
			}

		case GV_FLOAT:
			{
				return *(float*)mData < *(float*)other.mData;
			}

		case  GV_STRING:
			{
				return *(std::string*)mData < *(std::string*)other.mData;
			}

		case GV_NULL:
			{
				return false;
			}

		default:
			break;
		}
	}
	return false;
}
//---------------------------------------
bool GameVar::operator<=( const GameVar& other ) const
{
	bool result = false;
	if ( AreSameType( other, *this ) )
	{
		switch ( mType )
		{
		case GV_INT:
			{
				result = *(int*)mData <= *(int*)other.mData;
				break;
			}

		case GV_FLOAT:
			{
				result = *(float*)mData <= *(float*)other.mData;
				break;
			}

		case  GV_STRING:
			{
				result = *(std::string*)mData <= *(std::string*)other.mData;
				break;
			}

		case GV_NULL:
			{
				result = true;
				break;
			}

		default:
			break;
		}
	}
	return result;
}
//---------------------------------------
bool GameVar::operator> ( const GameVar& other ) const
{
	if ( AreSameType( other, *this ) )
	{
		switch ( mType )
		{
		case GV_INT:
			{
				return *(int*)mData > *(int*)other.mData;
			}

		case GV_FLOAT:
			{
				return *(float*)mData > *(float*)other.mData;
			}

		case  GV_STRING:
			{
				return *(std::string*)mData > *(std::string*)other.mData;
			}

		case GV_NULL:
			{
				return false;
			}

		default:
			break;
		}
	}
	return false;
}
//---------------------------------------
bool GameVar::operator>=( const GameVar& other ) const
{
	bool result = false;
	if ( AreSameType( other, *this ) )
	{
		switch ( mType )
		{
		case GV_INT:
			{
				result = *(int*)mData >= *(int*)other.mData;
				break;
			}

		case GV_FLOAT:
			{
				result = *(float*)mData >= *(float*)other.mData;
				break;
			}

		case  GV_STRING:
			{
				result = *(std::string*)mData >= *(std::string*)other.mData;
				break;
			}

		case GV_NULL:
			{
				result = true;
				break;
			}

		default:
			break;
		}
	}
	return result;
}
//---------------------------------------
