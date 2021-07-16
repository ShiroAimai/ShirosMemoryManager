#pragma once
#include <vector>
#include <memory>
#include "FixedAllocator.h"
#include "Mallocator.h"

constexpr size_t MAX_SMALL_OBJECT_SIZE = 64;


class ShirosSmallObjAllocator
{
public:
	ShirosSmallObjAllocator(size_t chunkSize);

	void* Allocate(size_t bytes);
	void Deallocate(void* p_obj, size_t size_obj);

	ShirosSmallObjAllocator(const ShirosSmallObjAllocator&) = delete;
	ShirosSmallObjAllocator& operator=(const ShirosSmallObjAllocator&) = delete;

	inline size_t GetTotalAllocatedMemory() const {	return m_totMemoryAllocated; }
private:
	using AllocatorPool = std::vector<FixedAllocator, Mallocator<FixedAllocator>>;
	AllocatorPool m_Pool;
	
	FixedAllocator* m_lastAllocatorUsedForAllocation = nullptr;
	FixedAllocator* m_lastAllocatorUsedForDeallocation = nullptr;
	size_t m_chunkSize;
	size_t m_totMemoryAllocated = 0;
};

