#include "MageLib.h"

using namespace mage;

//---------------------------------------
Dictionary::Dictionary()
{}
//---------------------------------------
Dictionary::Dictionary( const Dictionary& other )
{
	*this = other;
}
Dictionary::~Dictionary()
{
	for ( auto itr = mDictionary.begin(); itr != mDictionary.end(); ++itr )
	{
		Delete0( itr->second );
	}
}
//---------------------------------------