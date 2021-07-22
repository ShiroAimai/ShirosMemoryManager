#pragma once
#include <vector>
#include <memory>
#include "FixedAllocator.h"
#include "Mallocator.h"

constexpr size_t MAX_SMALL_OBJECT_SIZE = 128;


class SmallObjAllocator
{
public:
	SmallObjAllocator(const size_t chunkSize);

	void* Allocate(const size_t bytes);
	void Deallocate(void* p_obj, const size_t size_obj);

	void Reset();
	inline size_t GetTotalAllocatedMemory() const { return m_totMemoryAllocated; }

	SmallObjAllocator(const SmallObjAllocator&) = delete;
	SmallObjAllocator& operator=(const SmallObjAllocator&) = delete;
private:
	using AllocatorPool = std::vector<FixedAllocator, Mallocator<FixedAllocator>>;
	AllocatorPool m_Pool;
	
	FixedAllocator* m_lastAllocatorUsedForAllocation = nullptr;
	FixedAllocator* m_lastAllocatorUsedForDeallocation = nullptr;
	size_t m_chunkSize;
	size_t m_totMemoryAllocated = 0;
};

