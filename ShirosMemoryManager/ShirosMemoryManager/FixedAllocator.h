#pragma once
#include <vector>
#include "Mallocator.h"

constexpr size_t DEFAULT_CHUNK_SIZE = 4096;

class FixedAllocator
{
public:
	explicit FixedAllocator(size_t ChunkSize = 0, size_t BlockSize = 0);
	FixedAllocator(const FixedAllocator& other);
	FixedAllocator& operator=(const FixedAllocator& other);
	~FixedAllocator();

	void Swap(FixedAllocator& other);

	void* Allocate();
	void Deallocate(void* ptr);

	size_t GetBlockSize() const { return m_blockSize; }
private:
	/*Ensure Chunk is known only by a FixedAllocator*/
	struct Chunk
	{
		void Init(size_t blockSize, unsigned char blocks);
		void* Allocate(size_t blockSize);
		void Deallocate(void* p, size_t blockSize);
		void Reset(size_t blockSize, unsigned char blocks);
		void Release();
		unsigned char* m_data;
		unsigned char
			m_firstAvailableBlock,
			m_blocksAvailable;
	};

	void DeallocateImpl(void* ptr);
	Chunk* FindInVicinity(void* ptr);

	/*The fixed chunk's block size for this instance of FixedAllocator*/
	size_t m_blockSize;
	/*How many blocks a Chunk can contain*/
	unsigned char m_numBlocks;

	using Chunks = std::vector<Chunk, Mallocator<Chunk>>;
	/* All the chunks allocated for this instance of FixedAllocator*/
	Chunks m_chunks;
	/*The last chunk in which we allocated a block*/
	Chunk* m_lastChunkUsedForAllocation = nullptr;
	/*The last chunk in which we released a block*/
	Chunk* m_lastChunkUsedForDeallocation = nullptr;

	//boost and ensure copy semantics
	mutable const FixedAllocator* prev = nullptr;
	mutable const FixedAllocator* next = nullptr;
};

