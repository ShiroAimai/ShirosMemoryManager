#pragma once
#include <vector>
#include <memory>
#include "FixedAllocator.h"
#include "Mallocator.h"

constexpr size_t MAX_SMALL_OBJECT_SIZE = 128;


class SmallObjAllocator
{
public:
	SmallObjAllocator(size_t chunkSize);
	~SmallObjAllocator();

	/**
	 *	Allocates memory for SmallObjects
	 *
	 *@author Nicola Cisternino
	 *
	 *
	 *@param bytes - Requested allocation size
	 *@param OutAllocatedMemory - Effective memory size allocated
	 *
	 *@return A pointer to the memory address from which memory was allocated
	 * 
	 */
	void* Allocate(size_t bytes, size_t& OutAllocatedMemory);
	/**
	 *	Deallocates memory for SmallObjects
	 *
	 *@author Nicola Cisternino
	 *
	 *@param p_obj - Pointer to memory address from which deallocate
	 *@param size_obj - Requested size to be deallocated
	 *
	 *@return A pointer to the memory address from which memory was allocated
	 *
	 */
	size_t Deallocate(void* p_obj, size_t size_obj);

	void Reset();
	inline size_t GetTotalAllocatedMemory() const { return m_totMemoryAllocated; }

	/** Prevent copy for this class */
	SmallObjAllocator(const SmallObjAllocator&) = delete;
	SmallObjAllocator& operator=(const SmallObjAllocator&) = delete;
private:
	using AllocatorPool = std::vector<FixedAllocator, Mallocator<FixedAllocator>>;
	AllocatorPool m_Pool;
	
	FixedAllocator* m_lastAllocatorUsedForAllocation = nullptr;
	FixedAllocator* m_lastAllocatorUsedForDeallocation = nullptr;
	size_t m_chunkSize;
	
	//DEBUG
	size_t m_totMemoryAllocated = 0;
};

