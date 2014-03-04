/*
 * Author      : Matthew Johnson
 * Date        : 27/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	//---------------------------------------
	template< typename TReal >
	inline bool IsBitSet( const TReal& n, const uint32 bit )
	{
		// Note on !!:
		// The double not is for the case where
		// n is a float resulting in some close to zero result
		// to get this to an int result, negate to get 1.0 then again to get 0.0
		return !!( n & ( 1 << bit ) );
	}
	//---------------------------------------
	template< typename TReal >
	inline void SetBit( TReal& n, uint8 bit )
	{
		n |= ( 1 << bit );
	}
	//---------------------------------------
	template< typename TReal >
	inline void ClearBit( TReal& n, uint8 bit )
	{
		n &= !( 1 << bit );
	}
	//---------------------------------------
	template< typename TReal >
	inline void ToggleBit( TReal& n, uint8 bit )
	{
		n ^= 1 << bit;
	}
	//---------------------------------------

}