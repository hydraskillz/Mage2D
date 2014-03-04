#include "MageLib.h"

using namespace mage;

// Dirty h4x to deal with pre-main allocation and globals
// Ya, I could just make the memory pool initialize the first time new is called...
// But then I couldn't know when it's safe to free the memory pool.
static void* PRE_MAIN_ALLOCATIONS[ 64 ];	// Only 64 globals are allowed. Deal with it.
static uint8 PMA = 0;						// Pre-Main allocation count

// Some stuff to handle global pre-main allocation
#define PRE_MAIN_ALLOCATION( SIZE )														\
	printf( "Warning: Pre-main allocations not tracked: %u.\n", PMA );					\
	void* _ret = malloc( SIZE );														\
	PRE_MAIN_ALLOCATIONS[ PMA++ ] = _ret;												\
	return _ret;

// Handles freeing of pre-main allocations in an elegant macro
#define PRE_MAIN_FREE( MEM )															\
	for ( int i = 0; i < 64; ++i ) {													\
	if ( PRE_MAIN_ALLOCATIONS[ i ] == MEM ) {											\
	free( bytes ); if ( !--PMA ) MemoryPool::TerminateMemory(); return; }}


const uint32 MemoryPool::DefaultPoolSize      = 536870912U; // 512MB //1073741824U;	// 1GB (1GB caused out-of-memory issues sometimes)
const uint8 MemoryPool::BlockSize             = sizeof( MemoryPool::Block );
uint8* MemoryPool::Pool                       = NULL;
MemoryPool::Block* MemoryPool::Head           = NULL;
MemoryPool::Block* MemoryPool::NextFree		  = NULL;
uint32 MemoryPool::BlocksAllocated            = 0;
uint32 MemoryPool::BlocksFreed                = 0;
uint32 MemoryPool::TotalAllocationRequests    = 0;
uint32 MemoryPool::TotalBytesAllocated        = 0;
uint32 MemoryPool::LargestAllocationRequest   = 0;
uint32 MemoryPool::AverageAllocationRequested = 0;
uint32 MemoryPool::BlocksByUsage[ MEMUSAGE_COUNT ];

// For thread safety
static Mutex gMemoryMutex;

//---------------------------------------
bool MemoryPool::InitializeMemory()
{
	if ( Pool == NULL )
	{
		Pool = (uint8*) malloc( DefaultPoolSize );

		DebugAsssertion( Pool != NULL, "Failed to allocate memory pool.\n" );

		if ( Pool == NULL )
		{
			return false;
		}

		Head = (Block*) Pool;
		Head->BlockSize = DefaultPoolSize - BlockSize;
		Head->FileName = 0;
		Head->LineNumber = 0;
		Head->Free = true;
		Head->UserType = 0;
		Head->PrevBlock = NULL;
		Head->NextBlock = NULL;

		NextFree = Head;

		++BlocksFreed;

		BlocksByUsage[ Head->UserType ]++;
	}
	else
	{
		DebugAsssertion( false, "Memory pool already initialized!\n" );
	}
	return Pool != NULL;
}
//---------------------------------------
void MemoryPool::TerminateMemory()
{
	if ( Pool )
	{

		// Don't free the pool while there is a pre-main allocation
		if ( PMA > 0 ) return;

		// Report memory leaks
		Block* b = Head;
		Block* n = NULL;

		while ( b )
		{
			if ( !b->Free )
			{
				OutputDebugMessage( "%s(%u): Memory leak : %s.\n",
					b->FileName, b->LineNumber, ByteDisplay( b->BlockSize ).ToString() );
			}

			n = b;
			b = b->NextBlock;
		}

		// Free memory pool
		free( Pool );
		Pool = NULL;
	}
	else
	{
		DebugAsssertion( false, "Memory pool not initialized!\n Terminate has no effect." );
	}
}
//---------------------------------------
void* MemoryPool::Allocate( size_t bytes )
{
	return Allocate( NULL, 0, bytes );
}
//---------------------------------------
void* MemoryPool::Allocate( const char* filename, uint16 line, size_t bytes, uint8 usage )
{
	CriticalBlock( gMemoryMutex );

	// No memory Pool
	DebugAsssertion( Pool != NULL, "Allocate called before MemoryPool::Initialize()\n" );
	if ( Pool == NULL ) return NULL;
	// 0 byte allocation request
	if ( bytes == 0 ) return NULL;

	Block* b = NextFree;
	Block* n = NULL;

	// Just start at the head if the next free block won't fit
	if ( b->BlockSize < bytes ) b = Head;

	// Stats
	++TotalAllocationRequests;
	AverageAllocationRequested = (uint32) ( ( 0.9f * AverageAllocationRequested ) + ( 0.1f * bytes ) );
	if ( bytes > LargestAllocationRequest ) LargestAllocationRequest = bytes;

	// Search the linked list for a good spot to allocate
	// Layout:
	// NULL
	// Head
	// Block1
	// ...
	// BlockN
	// NULL

	while ( b )
	{
		// Found good block
		if ( b->Free && b->BlockSize >= bytes )
		{
			DebugAsssertion( b->BlockSize <= DefaultPoolSize, "BlockSize is larger than Memory Pool!\n" );

			uint32 lastSize = b->BlockSize;
			BlocksByUsage[ b->UserType ]--;
			b->BlockSize = bytes;
			b->FileName = filename;
			b->LineNumber = line;
			b->Free = false;
			b->UserType = usage;
			b->PrevBlock = n;

			++BlocksAllocated;
			--BlocksFreed;
			TotalBytesAllocated += bytes;
			BlocksByUsage[ usage ]++;

			Block* next = b->NextBlock;

			// Next is tail block, create new tail block
			// [ head ]    [ head ]
			// [ .... ] -> | tail |
			// [ tail ]    | tail |
			if ( next == NULL )
			{
				const uint32 sizeOfNextBlock = lastSize - b->BlockSize;
				b->NextBlock = (Block*) ( ( (uint8*) b ) + ( BlockSize + b->BlockSize ) );

				// Assert block is in pool
				DebugAsssertion( (uint8*) b->NextBlock > 0 && (uint8*) b->NextBlock < ( Pool + DefaultPoolSize ), "Tail overruns memory pool!\n" );

				next = b->NextBlock;
				next->BlockSize = sizeOfNextBlock - BlockSize;
				next->FileName = 0;
				next->LineNumber = 0;
				next->Free = true;
				next->UserType = 0;
				next->PrevBlock = b;
				next->NextBlock = NULL;

				NextFree = next;

				++BlocksFreed;
				BlocksByUsage[ next->UserType ]++;
				
				// Assert tail was successful
				DebugAsssertion( b->NextBlock->NextBlock == NULL, "Tail block is corrupt!\n" );
			}
			// Not tail block
			else
			{
				// Next is free, merge blocks
				// [ block ]    [ block ]
				// [ ----  ] -> | free  |
				// [ free  ]    | free  |
				if ( next->Free )
				{
					b->NextBlock = (Block*) ( (uint8*) b + ( BlockSize + bytes ) );

					Block* newNext = b->NextBlock;

					// Assert block is in pool
					DebugAsssertion( (uint8*) newNext > 0 && (uint8*) newNext < ( Pool + DefaultPoolSize ), "Bad merge in memory pool!\n" );

					newNext->BlockSize = (uint8*) newNext + BlockSize - (uint8*) next + next->BlockSize;
					newNext->FileName = 0;
					newNext->LineNumber = 0;
					newNext->Free = true;
					newNext->UserType = 0;
					newNext->PrevBlock = b;
					newNext->NextBlock = next->NextBlock;

					NextFree = newNext;

					BlocksByUsage[ newNext->UserType ]++;
				}
				// Insert free block
				// [ block ]    [ block ]
				// [ ----  ] -> [ free  ]
				// [ next  ]    [ next  ]
				else
				{
					uint32 freespace = lastSize - b->BlockSize;

					// A new block can be inserted between current and next
					if ( freespace > BlockSize )
					{
						b->NextBlock = (Block*) ( ( (uint8*) b ) + ( BlockSize + b->BlockSize ) );
						
						Block* newNext = b->NextBlock;

						// Assert block is in pool
						DebugAsssertion( (uint8*) newNext > 0 && (uint8*) newNext < ( Pool + DefaultPoolSize ), "Bad insertion in memory pool!\n" );

						newNext->BlockSize = freespace - BlockSize;
						newNext->FileName = 0;
						newNext->LineNumber = 0;
						newNext->Free = true;
						newNext->UserType = 0;
						newNext->PrevBlock = b;
						newNext->NextBlock = next;

						NextFree = newNext;

						++BlocksFreed;
						BlocksByUsage[ newNext->UserType ]++;
					}
					// else Not enough memory (do nothing)
				}
			}
			uint8* _ret = (uint8*) b + BlockSize;
			// Assert block is in pool
			DebugAsssertion( _ret > 0 && _ret < ( Pool + DefaultPoolSize ), "Access violation in memory pool!\n" );
			return _ret;
		}

		n = b;
		b = b->NextBlock;
	}

	throw std::bad_alloc( "Out of memory. Try increasing default memory pool size." );

	// Not reachable but w/e
	return NULL;
}
//---------------------------------------
void MemoryPool::Free( void* memory )
{
	CriticalBlock( gMemoryMutex );

	// Do nothing if memory is null
	if ( !memory ) return;
	if ( !Pool ) return;

	Block* b = (Block*) ( (uint8*) memory - BlockSize );
	// Assert block is in pool
	DebugAsssertion( (uint8*)b > 0 && (uint8*)b < ( Pool + DefaultPoolSize ), "Access violation in memory pool!\n" );

	Block* prev = b->PrevBlock;
	Block* next = b->NextBlock;

	BlocksByUsage[ b->UserType ]--;
	b->FileName = 0;
	b->LineNumber = 0;
	b->Free = true;
	b->UserType = 0;

	NextFree = b;

	// Stats!
	++BlocksFreed;
	--BlocksAllocated;
	TotalBytesAllocated -= b->BlockSize;
	BlocksByUsage[ b->UserType ]++;

	// 3 way merge
	// [ prev  ]    | prev |
	// [ b     ] -> | prev |
	// [ next  ]    | prev |
	if ( prev && next && prev->Free && next->Free )
	{
		BlocksByUsage[ b->UserType ]--;
		BlocksByUsage[ next->UserType ]--;

		prev->NextBlock = next->NextBlock;
		if ( next->NextBlock )
			next->NextBlock->PrevBlock = prev;
		prev->BlockSize = prev->BlockSize + next->BlockSize + b->BlockSize + 2 * BlockSize;

		NextFree = prev;

		// Assert block is in pool
		DebugAsssertion( (uint8*)prev > 0 && (uint8*)prev < ( Pool + DefaultPoolSize ), "Access violation in memory pool!\n" );

		BlocksFreed -= 2;
	}
	else
	// Merge with previous block
	// [ prev  ]    | prev |
	// [ b     ] -> | prev |
	// [ next  ]    [ next ]
	if ( prev && prev->Free )
	{
		BlocksByUsage[ b->UserType ]--;
		
		prev->BlockSize = prev->BlockSize + BlockSize + b->BlockSize;
		prev->NextBlock = next;
		next->PrevBlock = prev;

		NextFree = prev;

		// Assert block is in pool
		DebugAsssertion( (uint8*)prev > 0 && (uint8*)prev < ( Pool + DefaultPoolSize ), "Access violation in memory pool!\n" );

		--BlocksFreed;
	}
	else
	// Merge with next block
	// [ prev  ]    [ prev ]
	// [ b     ] -> | b    |
	// [ next  ]    | b    |
	if ( next && next->Free )
	{
		BlocksByUsage[ next->UserType ]--;

		b->BlockSize = b->BlockSize + BlockSize + next->BlockSize;
		b->NextBlock = next->NextBlock;
		if ( b->NextBlock )
			b->NextBlock->PrevBlock = b;

		// Assert block is in pool
		DebugAsssertion( (uint8*)b > 0 && (uint8*)b < ( Pool + DefaultPoolSize ), "Access violation in memory pool!\n" );

		--BlocksFreed;
	}
}
//---------------------------------------
void MemoryPool::Free( void* memory, const char*, uint16 )
{
	Free( memory );
}
//---------------------------------------
uint32 MemoryPool::GetFreeBlockCount()
{
	return BlocksFreed;
}
//---------------------------------------
uint32 MemoryPool::GetAllocatedBlockCount()
{
	return BlocksAllocated;
}
//---------------------------------------
uint32 MemoryPool::GetTotalAllocationRequests()
{
	return TotalAllocationRequests;
}
//---------------------------------------
uint32 MemoryPool::GetTotalBytesAllocated()
{
	return TotalBytesAllocated;
}
//---------------------------------------
uint32 MemoryPool::GetLargestAllocationRequested()
{
	return LargestAllocationRequest;
}
//---------------------------------------
uint32 MemoryPool::GetAverageAllocationRequest()
{
	return AverageAllocationRequested;
}
//---------------------------------------



//---------------------------------------
// ByteDisplay
const char* ByteDisplay::msSuffixes[] =
{
	"B",
	"KB",
	"MB",
	"GB"
};
//---------------------------------------
ByteDisplay::ByteDisplay( uint32 bytes, ByteDisplaySize size )
	: mSuffixIndex( 0 )
	, Amount( bytes )
	, Suffix( msSuffixes[0] )
{
	if ( size == SIZE_AUTO )
	{
		SetToAuto();
	}
}
//---------------------------------------
void ByteDisplay::SetToAuto()
{
	double bytes = Amount;
	while ( bytes > 1024 )
	{
		bytes /= 1024;
		++mSuffixIndex;
	}
	Suffix = msSuffixes[ mSuffixIndex ];
	sprintf_s( mString, "%.*f %s", mSuffixIndex > 0 ? 2 : 0, bytes, Suffix );
}
//---------------------------------------
const char* ByteDisplay::ToString() const
{
	return mString;
}
//---------------------------------------


//---------------------------------------
// MemUsageDisplay
const char* MemUsageDisplay::USAGE_NAMES[] =
{
	"None",
	"General",
	"Audio",
	"Texture",
	"Allocator",
	"Custom"
};
//---------------------------------------


#ifdef USE_MEMORY_MANAGER

#pragma push_macro( "new" )
#undef new


//---------------------------------------
// New overrides
void* operator new( size_t bytes, const char* filename, mage::uint16 line ) throw( ... )
{
	if ( mage::MemoryPool::Pool == NULL )
	{
		PRE_MAIN_ALLOCATION( bytes );
	}
	return mage::MemoryPool::Allocate( filename, line, bytes );
}
//---------------------------------------
void* operator new[]( size_t bytes, const char* filename, mage::uint16 line ) throw( ... )
{
	if ( mage::MemoryPool::Pool == NULL )
	{
		PRE_MAIN_ALLOCATION( bytes );
	}
	return mage::MemoryPool::Allocate( filename, line, bytes );
}
//---------------------------------------
void* operator new( size_t bytes ) throw( ... )
{
	if ( mage::MemoryPool::Pool == NULL )
	{
		PRE_MAIN_ALLOCATION( bytes );
	}
	return mage::MemoryPool::Allocate( bytes );
}
//---------------------------------------
void* operator new[]( size_t bytes ) throw( ... )
{
	if ( mage::MemoryPool::Pool == NULL )
	{
		PRE_MAIN_ALLOCATION( bytes );
	}
	return mage::MemoryPool::Allocate( bytes );
}
//---------------------------------------
// Delete overrides
void operator delete( void* bytes, const char* filename, mage::uint16 line )
{
	PRE_MAIN_FREE( bytes );
	mage::MemoryPool::Free( bytes, filename, line );
}
//---------------------------------------
void operator delete[]( void* bytes, const char* filename, mage::uint16 line )
{
	PRE_MAIN_FREE( bytes );
	mage::MemoryPool::Free( bytes, filename, line );
}
//---------------------------------------
void operator delete( void* bytes )
{
	PRE_MAIN_FREE( bytes );
	mage::MemoryPool::Free( bytes );
}
//---------------------------------------
void operator delete[]( void* bytes )
{
	PRE_MAIN_FREE( bytes );
	mage::MemoryPool::Free( bytes );
}
//---------------------------------------

#pragma pop_macro( "new" )

#endif