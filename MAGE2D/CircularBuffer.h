/*
 * Author      : Matthew Johnson
 * Date        : 26/Jan/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	template< typename T >
	class CircularBuffer
	{
	public:
		CircularBuffer( int size=512 );
		~CircularBuffer();

		inline bool IsEmpty() const;
		void Write( const T& element );
		const T& Read( int offset=0 ) const;
		void Resize( int newSize );
		void Clear();
		int Length() const;

	private:
		int mSize;
		int mStart;
		int mEnd;
		T* mElements;
	};

	//---------------------------------------
	// Implementation
	//---------------------------------------
	
	//---------------------------------------
	template< typename T >
	CircularBuffer< T >::CircularBuffer( int size )
		: mSize( size+1 )
		, mStart( 0 )
		, mEnd( 0 )
	{
		mElements = new T[ mSize ];
	}
	//---------------------------------------
	template< typename T >
	CircularBuffer< T >::~CircularBuffer()
	{
		delete[] mElements;
	}
	//---------------------------------------
	template< typename T >
	inline bool CircularBuffer< T >::IsEmpty() const
	{
		return mEnd == mStart;
	}
	//---------------------------------------
	template< typename T >
	void CircularBuffer< T >::Write( const T& element )
	{
		mElements[ mEnd ] = element;
		mEnd = ( mEnd + 1 ) % mSize;
		// Buffer is full, overwrite
		if ( mEnd == mStart )
		{
			mStart = ( mStart + 1 ) % mSize;
		}
	}
	//---------------------------------------
	template< typename T >
	const T& CircularBuffer< T >::Read( int offset=0 ) const
	{
		int index = ( mStart + offset ) % mSize;
		return mElements[ index ];
	}
	//---------------------------------------
	template< typename T >
	void CircularBuffer< T >::Resize( int newSize )
	{
		T* newBuffer = new T[ newSize+1 ];
		memcpy( newBuffer, mElements, sizeof( T ) * mSize );
		delete[] mElements;
		mElements = newBuffer;
		mSize = newSize+1;
	}
	//---------------------------------------
	template< typename T >
	void CircularBuffer< T >::Clear()
	{
		mEnd = 0;
		mStart = 0;
	}
	//---------------------------------------
	template< typename T >
	int CircularBuffer< T >::Length() const
	{
		if ( mStart > mEnd ) return mSize - 1;
		return mStart + mEnd;
	}
	//---------------------------------------

}