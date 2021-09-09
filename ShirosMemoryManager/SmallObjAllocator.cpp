#include "pch.h"
#include "SmallObjAllocator.h"

namespace {
	auto FixedAllocatorComparator = [](const FixedAllocator& Allocator, size_t bytes) { return Allocator.GetBlockSize() < bytes; };
}

SmallObjAllocator::SmallObjAllocator(size_t chunkSize)
	: m_chunkSize(chunkSize)
{

}

SmallObjAllocator::~SmallObjAllocator()
{
	AllocatorPool::iterator it = m_Pool.begin();
	for (; it != m_Pool.end(); ++it)
	{
		it->Release();
	}
}

void* SmallObjAllocator::Allocate(size_t bytes, size_t& OutAllocatedMemory)
{
	OutAllocatedMemory = bytes; //we allocate just the right amount of memory

	//check if we already allocated an allocator, and if this one manage Chunk of the desired size
	if (!m_lastAllocatorUsedForAllocation || m_lastAllocatorUsedForAllocation->GetBlockSize() != bytes)
	{
		//find the correct allocator in list
		AllocatorPool::iterator it = std::lower_bound(m_Pool.begin(), m_Pool.end(), bytes, FixedAllocatorComparator);

		// if the allocator that manage this size_t is nowhere to be found 
		// or the one we found does not manage the exact sizeof bytes
		// then insert a new Allocator that'll do the work
		if (it == m_Pool.end() || it->GetBlockSize() != bytes)
		{
			it = m_Pool.insert(it, FixedAllocator(m_chunkSize, bytes));
			m_lastAllocatorUsedForDeallocation = &*m_Pool.begin();
		}
		
		//either we found an allocator or we're using the one we just inserted
		m_lastAllocatorUsedForAllocation = &*it;
	}

	m_totMemoryAllocated -= m_lastAllocatorUsedForAllocation->GetTotalAllocatedMemory(); //remove current memory occupied by it that does not consider new allocation
	void* p_res = m_lastAllocatorUsedForAllocation->Allocate();
	m_totMemoryAllocated += m_lastAllocatorUsedForAllocation->GetTotalAllocatedMemory(); //add new value for memory occupied by it also considering new allocation
	return p_res;
}

size_t SmallObjAllocator::Deallocate(void* p_obj, size_t size_obj)
{
	//check if the last time we deallocated something from an allocator, and if that allocator is of the desired size
	//if it is, then use that one
	if (m_lastAllocatorUsedForDeallocation && m_lastAllocatorUsedForDeallocation->GetBlockSize() == size_obj)
	{
		m_lastAllocatorUsedForDeallocation->Deallocate(p_obj);
		return size_obj;
	}
	//find the allocator used to allocate the object requested to release
	AllocatorPool::iterator it = std::lower_bound(m_Pool.begin(), m_Pool.end(), size_obj, FixedAllocatorComparator);
	//assert the allocator exists and it is of the right size
	//it MUST be impossible to delete an object that was previously allocated using our Allocator!
	assert(it != m_Pool.end());
	assert(it->GetBlockSize() == size_obj);
	m_lastAllocatorUsedForDeallocation = &*it;
	m_lastAllocatorUsedForDeallocation->Deallocate(p_obj);

	return size_obj; //we deallocate just the right amount
}

void SmallObjAllocator::Reset()
{
	m_totMemoryAllocated = 0;

	m_lastAllocatorUsedForDeallocation = nullptr;
	m_lastAllocatorUsedForAllocation = nullptr;

	AllocatorPool::iterator it = m_Pool.begin();
	for (; it != m_Pool.end(); ++it)
	{
		it->Release();
	}

	m_Pool.clear();
}
