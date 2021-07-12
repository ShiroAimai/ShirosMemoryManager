#pragma once
#include <vector>
#include "FixedAllocator.h"

typedef size_t;

constexpr size_t MAX_SMALL_OBJECT_SIZE = 64;

class ShirosSmallObjAllocator
{
public:
	ShirosSmallObjAllocator(size_t chunkSize);

	void* Allocate(size_t bytes);
	void Deallocate(void* p_obj, size_t size_obj);

	ShirosSmallObjAllocator(const ShirosSmallObjAllocator&) = delete;
	ShirosSmallObjAllocator& operator=(const ShirosSmallObjAllocator&) = delete;
private:
	using AllocatorPool = std::vector<FixedAllocator>;
	AllocatorPool m_Pool;

	FixedAllocator* m_lastAllocatorUsedForAllocation;
	FixedAllocator* m_lastAllocatorUsedForDeallocation;
	size_t m_chunkSize;
};

