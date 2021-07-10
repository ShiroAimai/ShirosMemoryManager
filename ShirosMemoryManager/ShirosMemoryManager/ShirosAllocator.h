#pragma once
#include <vector>
#include "FixedAllocator.h"

typedef size_t;

constexpr size_t MAX_SMALL_OBJECT_SIZE = 64;

class ShirosAllocator
{
public:
	ShirosAllocator(size_t chunkSize, size_t maxSize);

	void* Allocate(size_t bytes);
	void Deallocate(void* p_obj, size_t size_obj);

	ShirosAllocator(const ShirosAllocator&) = delete;
	ShirosAllocator& operator=(const ShirosAllocator&) = delete;
private:
	using AllocatorPool = std::vector<FixedAllocator>;
	AllocatorPool m_Pool;

	FixedAllocator* m_lastAllocatorUsedForAllocation;
	FixedAllocator* m_lastAllocatorUsedForDeallocation;
	size_t m_chunkSize;
	size_t m_maxAllowedObjectSize;
};

