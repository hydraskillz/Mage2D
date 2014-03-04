/*
 * Author      : Matthew Johnson
 * Date        : 21/Jun/2013
 * Description :
 *   Custom memory manager.
 */
 
#pragma once

//@TODO file needs general clean up/organization

// Comment out to disable global override of memory manager
//#define USE_MEMORY_MANAGER

// Memory Tags
typedef enum : mage::uint8
{
	MEMUSAGE_GENERAL = 1,
	MEMUSAGE_AUDIO,
	MEMUSAGE_TEXTURE,
	MEMUSAGE_ALLOCATOR,
	MEMUSAGE_CUSTOM,
	MEMUSAGE_COUNT
} MageMemoryUsageTag;

// Utility to display MageMemoryUsageTag as a string
namespace mage
{
	struct MemUsageDisplay
	{
	private:
		static const char* USAGE_NAMES[];
		uint8 mTag;
	public:
		MemUsageDisplay( uint8 tag ) : mTag( tag ) {}
		const char* ToString() const
		{
			return mTag < MEMUSAGE_COUNT ? USAGE_NAMES[ mTag ] : "General";
		}
	};
}

// MageMemory allocation macros
#define mage_allocate( SIZE_IN_BYTES )									\
	MemoryPool::Allocate( __FILE__, __LINE__, SIZE_IN_BYTES )

#define mage_allocate_tagged( SIZE_IN_BYTES, MEM_USAGE_TAG )			\
	MemoryPool::Allocate( __FILE__, __LINE__, SIZE_IN_BYTES, MEM_USAGE_TAG )

#define mage_free( memory )												\
	MemoryPool::Free( memory )

// @TODO remove this it's stupid
#define New0 new

// Delete and zero pointer
#define Delete0( ptr ) if ( ptr != 0 ) { delete ptr; ptr = 0; }
#define Delete1( ptr ) if ( ptr != 0 ) { delete[] ptr; ptr = 0; }

// Call delete on all values in a std::map then clear() the map
#define DestroyMapByValue( MAP )										\
	for ( auto itr = MAP.begin(); itr != MAP.end(); ++itr )				\
	{																	\
		delete itr->second;												\
	}																	\
	MAP.clear();

// Call delete on all items in the std::vector and clear() the vector
#define DestroyVector( VECTOR )											\
	for ( auto itr = VECTOR.begin(); itr != VECTOR.end(); ++itr )		\
	{																	\
		Delete0( *itr );												\
	}																	\
	VECTOR.clear();

namespace mage
{

	//---------------------------------------
	// ByteDisplay
	// Utility class for displaying byte data
	struct ByteDisplay
	{
	private:
		static const char* msSuffixes[];
		uint8 mSuffixIndex;
		char mString[20];
	public:
		enum ByteDisplaySize
		{
			SIZE_AUTO,		// Pick best fit size
			BYTES,
			KILOBYTES,
			MEGEBYTES,
			GIGABYTES
		};
		ByteDisplay( uint32 bytes, ByteDisplaySize size=SIZE_AUTO );

		void SetToAuto();
		const char* ToString() const;

		const char* Suffix;
		uint32 Amount;
	};
	//---------------------------------------


	//---------------------------------------
	class MemoryPool
	{
	public:
		struct Block 
		{
			Block* PrevBlock;			// 4
			Block* NextBlock;			// 4
			uint32 BlockSize;			// 4
			const char* FileName;		// 4
			uint16 LineNumber;			// 2
			bool Free;					// 1
			uint8 UserType;				// 1
		}; // 20 bytes

		// Only ever call this once
		static bool InitializeMemory();
		// Call before exit, it will be delayed if globals are waiting to be freed
		static void TerminateMemory();
		static void* Allocate( size_t bytes );
		static void* Allocate( const char* filename, uint16 line, size_t bytes, uint8 usage=MEMUSAGE_GENERAL );
		static void Free( void* memory );
		static void Free( void* memory, const char*, uint16 );

		// Statistical info
		static uint32 GetFreeBlockCount();
		static uint32 GetAllocatedBlockCount();
		static uint32 GetTotalAllocationRequests();
		static uint32 GetTotalBytesAllocated();
		static uint32 GetLargestAllocationRequested();
		static uint32 GetAverageAllocationRequest();

//	private:	// Public for now... easier to display debug info
		static const uint32 DefaultPoolSize;
		static const uint8 BlockSize;
		static uint8* Pool;
		static Block* Head;								// (Block*) Pool
		static Block* NextFree;							// Last free block encountered
		static uint32 BlocksAllocated;					// Total allocated block count
		static uint32 BlocksFreed;
		static uint32 BlocksByUsage[ MEMUSAGE_COUNT ];	// Blocks by usage tag
		static uint32 TotalAllocationRequests;
		static uint32 TotalBytesAllocated;
		static uint32 LargestAllocationRequest;
		static uint32 AverageAllocationRequested;		// Running average
	};

}

// New/Delete overrides
#ifdef USE_MEMORY_MANAGER

#pragma push_macro( "new" )
#undef new

//---------------------------------------
// New overrides
void* operator new( size_t bytes, const char* filename, mage::uint16 line ) throw( ... );
//---------------------------------------
void* operator new[]( size_t bytes, const char* filename, mage::uint16 line ) throw( ... );
//---------------------------------------
void* operator new( size_t bytes ) throw( ... );
//---------------------------------------
void* operator new[]( size_t bytes ) throw( ... );
//---------------------------------------
// Delete overrides
void operator delete( void* bytes, const char* filename, mage::uint16 line );
//---------------------------------------
void operator delete[]( void* bytes, const char* filename, mage::uint16 line );
//---------------------------------------
void operator delete( void* bytes );
//---------------------------------------
void operator delete[]( void* bytes );
//---------------------------------------

#pragma pop_macro( "new" )

#endif

// Test allocator for use in std containers
// Based off an allocator example from some book
#include <limits>
namespace mage
{
	template< typename T >
	class MageAlloc
	{
	public:

		typedef T				value_type;
		typedef T*				pointer;
		typedef const T*		const_pointer;
		typedef T&				reference;
		typedef const T&		const_reference;
		typedef std::size_t		size_type;
		typedef std::ptrdiff_t	difference_type;

		template< typename U >
		struct rebind
		{
			typedef MageAlloc< U > other;
		};

		pointer address( reference value ) const { return &value; }
		const_pointer address( const_reference value ) const { return &value; }

		MageAlloc() throw() {}
		MageAlloc( const MageAlloc& ) throw() {}

		template< typename U >
		MageAlloc( const MageAlloc< U >& ) throw() {}
		~MageAlloc() throw() {}

		size_type max_size() const throw() { return std::numeric_limits< std::size_t >::max() / sizeof( T ); }

		pointer allocate( size_type num, const void* = 0 )
		{
			return (pointer) ( mage_allocate_tagged( num * sizeof( T ), MEMUSAGE_ALLOCATOR ) );
		}

		void construct( pointer p, const_reference value )
		{
			new( (void*) p ) T(value);
		}

		void destroy( pointer p )
		{
			p->~T();
		}

		void deallocate( pointer p, size_type num )
		{
			mage_free( (void*) p );
		}
	};

	template< typename T1, typename T2 >
	bool operator==( const MageAlloc< T1 >&, const MageAlloc< T2 >& ) throw() { return true; }
	template< typename T1, typename T2 >
	bool operator!=( const MageAlloc< T1 >&, const MageAlloc< T2 >& ) throw() { return false; }
}