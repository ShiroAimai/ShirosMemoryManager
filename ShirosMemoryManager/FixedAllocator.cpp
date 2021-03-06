#include "pch.h"
#include "FixedAllocator.h"

void FixedAllocator::Chunk::Init(size_t blockSize, unsigned char blocks)
{
	assert(blockSize > 0); //block size MUST be greater than 0, min. 1
	assert(blocks > 0); //chunk must be composed of at least one element (i.e an element of max size)
	assert((blockSize * blocks) / blockSize == blocks); // check for overflow

	m_data = static_cast<unsigned char*>(std::malloc(blockSize * blocks)); //reserve heap memory for the chunk. 
	Reset(blockSize, blocks);
}

void* FixedAllocator::Chunk::Allocate(size_t blockSize)
{
	if (m_blocksAvailable == 0) return nullptr;

	assert((m_firstAvailableBlock * blockSize) / blockSize == m_firstAvailableBlock); //overflow check
	
	unsigned char* result = m_data + (m_firstAvailableBlock * blockSize); //simple arithmetic operation to find new block start
	m_firstAvailableBlock = *result; //copy index of next available block contained in result
	
	--m_blocksAvailable; //decrement available blocks in chunk
	
	return result;
}

void FixedAllocator::Chunk::Deallocate(void* p, size_t blockSize)
{
	assert(p >= m_data); //ensure ptr is greater or equal to the first address contained in m_data

	unsigned char* toDealloc = static_cast<unsigned char*>(p);

	//Address must be aligned to blockSize, so module should evaluate to 0
	assert((toDealloc - m_data) % blockSize == 0); //alignment check to blockSize provided in input. 

	*toDealloc = m_firstAvailableBlock; //ovverride toDealloc block value with current index of m_firstAvailableBlock
	//assign the index of the just deallocated block as m_firstAvailableBlock
	m_firstAvailableBlock = static_cast<unsigned char>((toDealloc - m_data) / blockSize); 

	assert(m_firstAvailableBlock == (toDealloc - m_data) / blockSize);

	++m_blocksAvailable;
}

void FixedAllocator::Chunk::Reset(size_t blockSize, unsigned char blocks)
{
	assert(blockSize > 0); //block size MUST be greater than 0, min. 1
	assert(blocks > 0); //chunk must be composed of at least one element (i.e an element of max size)
	assert((blockSize * blocks) / blockSize == blocks); // check for overflow

	m_firstAvailableBlock = 0; // reset to first block available
	m_blocksAvailable = blocks; //all blocks available

	unsigned char* p_temp = m_data;
	for (unsigned char i = 0; i != blocks; p_temp += blockSize)
	{
		//each block starting point has the index of the block that follows
		*p_temp = ++i; //re-assign memory to override eventual garbage
	}
}

void FixedAllocator::Chunk::Release()
{
	assert(m_data != nullptr);
	std::free(m_data);
}

FixedAllocator::FixedAllocator(size_t ChunkSize /*= 0*/,size_t BlockSize /*= 0*/)
	: m_blockSize(BlockSize),
	m_lastChunkUsedForAllocation(nullptr),
	m_lastChunkUsedForDeallocation(nullptr)
{
	assert(BlockSize > 0); //ensure you're not creating an allocator of size 0, min. 1
	
	prev = next = this; 

	//if input ChunkSize is greater than 0 use that, otherwise fallback
	size_t AllocatorChunkSize = ChunkSize > 0 ? ChunkSize : DEFAULT_CHUNK_SIZE; 
	//compute effective number of blocks per chunk
	size_t numBlocks = AllocatorChunkSize / BlockSize; 
	
	if (numBlocks > UCHAR_MAX)
	{
		numBlocks = UCHAR_MAX; //cap chunk block to the value of UCHAR_MAX(255)
	}
	else if (numBlocks == 0)
	{
		numBlocks = CHAR_BIT * BlockSize; //fallback
	}

	m_numBlocks = static_cast<unsigned char>(numBlocks);
	assert(m_numBlocks == numBlocks); //validate assignment 
}

FixedAllocator::FixedAllocator(const FixedAllocator& other)
	: m_blockSize(other.m_blockSize),
	m_numBlocks(other.m_numBlocks),
	m_chunks(other.m_chunks)
{
	//insert constructed Allocator between other and the previous element following other
	prev = &other;
	next = other.next;
	other.next->prev = this;
	other.next = this;

	//compute local address for last chunk used for allocation using the offset of other.m_lastChunkUsedForAllocation address from other starting address
	m_lastChunkUsedForAllocation = other.m_lastChunkUsedForAllocation
		? &m_chunks.front() + (other.m_lastChunkUsedForAllocation - &other.m_chunks.front())
		: 0;
	//compute local address for last chunk used for deallocation using the offset of other.m_lastChunkUsedForDeallocation address from other starting address
	m_lastChunkUsedForDeallocation = other.m_lastChunkUsedForDeallocation
		? &m_chunks.front() + (other.m_lastChunkUsedForDeallocation - &other.m_chunks.front())
		: 0;
}

FixedAllocator& FixedAllocator::operator=(const FixedAllocator& other)
{
	FixedAllocator copy(other);
	copy.Swap(*this);
	return *this;
}

FixedAllocator::~FixedAllocator()
{
	//if allocator is not already detached from other FixedAllocators in list
	if (prev != this || next != this)
	{
		//assign FixedAllocator that follows this to previous one
		if(prev)
		{
			prev->next = next;
		}
		//assign FixedAllocator that precedes this to next one
		if (next)
		{
			next->prev = prev;
		}
		
		prev = next = nullptr;
		return;
	}

	assert(prev == next); //detached fixed allocator
	
	//release all chunks for this FixedAllocator
	for (Chunks::iterator it = m_chunks.begin(); it != m_chunks.end(); ++it)
	{
		it->Release();
	}
}

void FixedAllocator::Swap(FixedAllocator& other)
{
	using std::swap;
	swap(m_blockSize, other.m_blockSize);
	swap(m_numBlocks, other.m_numBlocks);
	m_chunks.swap(other.m_chunks);
	swap(m_lastChunkUsedForAllocation, other.m_lastChunkUsedForAllocation);
	swap(m_lastChunkUsedForDeallocation, other.m_lastChunkUsedForDeallocation);
}

void* FixedAllocator::Allocate()
{
	if (m_lastChunkUsedForAllocation == 0 || m_lastChunkUsedForAllocation->m_blocksAvailable == 0)
	{
		for (Chunks::iterator it = m_chunks.begin();; ++it)
		{
			if (it == m_chunks.end())
			{
				//append new chunk
				//reserve memory for all the already present chunks and also for the new one
				m_chunks.reserve(m_chunks.size() + 1);
				
				Chunk NewChunk;
				NewChunk.Init(m_blockSize, m_numBlocks);
				
				m_chunks.push_back(NewChunk);
				m_lastChunkUsedForAllocation  = &m_chunks.back();
				m_lastChunkUsedForDeallocation = &m_chunks.front();
				
				break;
			}

			if (it->m_blocksAvailable > 0)
			{
				m_lastChunkUsedForAllocation = &*it;
				break;
			}
		}
	}
	
	assert(m_lastChunkUsedForAllocation != 0);
	assert(m_lastChunkUsedForAllocation->m_blocksAvailable > 0);

	return m_lastChunkUsedForAllocation->Allocate(m_blockSize);
}

void FixedAllocator::Deallocate(void* ptr)
{
	assert(!m_chunks.empty());
	assert(&m_chunks.front() <= m_lastChunkUsedForDeallocation);
	assert(&m_chunks.back() >= m_lastChunkUsedForDeallocation);

	m_lastChunkUsedForDeallocation = FindInVicinity(ptr);
	assert(m_lastChunkUsedForDeallocation);

	DeallocateImpl(ptr);
}

void FixedAllocator::Release()
{
	//clear memory allocated for this FixedAllocator chunks
	for (Chunks::iterator it = m_chunks.begin(); it != m_chunks.end(); ++it)
	{
		it->Release();
	}
	m_chunks.clear(); //remove all chunks

	//reset addresses
	m_lastChunkUsedForAllocation = nullptr;
	m_lastChunkUsedForDeallocation = nullptr;

	prev = nullptr;
	next = nullptr;
}

void FixedAllocator::DeallocateImpl(void* ptr)
{
	//assert ptr is a memory address between the Chunk first block and the Chunk last possible memory address
	assert(m_lastChunkUsedForDeallocation->m_data <= ptr); 
	assert(m_lastChunkUsedForDeallocation->m_data + (m_numBlocks * m_blockSize) > ptr);

	//we're not releasing memory here, only clearing the block pointed by ptr 
	m_lastChunkUsedForDeallocation->Deallocate(ptr, m_blockSize);

	if (m_lastChunkUsedForDeallocation->m_blocksAvailable == m_numBlocks) //Last Chunk used for deallocation now is empty
	{
		//shall we release this chunk then?
		//We release the chunk only if we find at least two empty Chunks

		Chunk& lastChunkInList = m_chunks.back();

		//if the last chunk in list is also the one from which we just removed a block
		if (&lastChunkInList == m_lastChunkUsedForDeallocation)
		{
			//we have to make sure there are two empty Chunks
			//check list is not empty and also
			//check previous chunk to m_lastChunkUsedForDeallocation is empty
			if (m_chunks.size() > 1 && (m_lastChunkUsedForDeallocation - 1)->m_blocksAvailable == m_numBlocks)
			{
				m_lastChunkUsedForDeallocation->Release();
				m_chunks.pop_back();
				//reset pointers to a stable situation
				m_lastChunkUsedForAllocation = m_lastChunkUsedForDeallocation = &m_chunks.front();
			}

			return; //we're done
		}

		//if not then check the last chunk in list anyway
		if (lastChunkInList.m_blocksAvailable == m_numBlocks)
		{
			//if it is empty then we got two non-contiguous empty Chunk, release the last one in list
			lastChunkInList.Release();
			m_chunks.pop_back();
			m_lastChunkUsedForAllocation = m_lastChunkUsedForDeallocation;
		}
		else
		{
			//we didn't find a second empty chunk, swap this empty chunk with the last in list
			std::swap(*m_lastChunkUsedForDeallocation, lastChunkInList);
			m_lastChunkUsedForAllocation = &m_chunks.back();
		}
	}
}

FixedAllocator::Chunk* FixedAllocator::FindInVicinity(void* ptr)
{
	assert(!m_chunks.empty());
	assert(m_lastChunkUsedForDeallocation);

	const size_t ChunkMaxCapacity = m_numBlocks * m_blockSize;

	//start to search in the interval [front, back + 1]
	Chunk* left = m_lastChunkUsedForDeallocation;
	Chunk* right = m_lastChunkUsedForDeallocation + 1;
	Chunk* left_bound = &m_chunks.front();
	Chunk* right_bound = &m_chunks.back() + 1;

	if (right == right_bound)
		right = 0;

	for (;;)
	{
		if (left == 0 && right == 0) //if the ptr has not been found exit
			break;

		//check if ptr is pointing to a block inside a Chunk that 
		//is equal or precedes the m_lastChunkUsedForDeallocation
		if (left)
		{
			if (ptr >= left->m_data && ptr < left->m_data + ChunkMaxCapacity)
			{
				return left;
			}
			if (left == left_bound) //we're at the start of the Chunk
				left = 0;
			else --left;
		}

		if (right)
		{
			if (ptr >= right->m_data && ptr < right->m_data + ChunkMaxCapacity)
			{
				return right;
			}
			++right;
			if (right == right_bound) //we're at the end of the Chunk
				right = 0;
		}
	}

	assert(false); //signal that there's a problem if we ended up here
	return 0;
}
