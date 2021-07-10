#include "pch.h"
#include "ShirosAllocator.h"
#include "FixedAllocator.h"

namespace {
	auto FixedAllocatorComparator = [](const FixedAllocator& Allocator, size_t bytes) { return Allocator.GetBlockSize() == bytes; };
}
ShirosAllocator::ShirosAllocator(size_t chunkSize, size_t maxSize)
	: m_chunkSize(chunkSize), m_maxAllowedObjectSize(maxSize),
	m_lastAllocatorUsedForDeallocation(nullptr), m_lastAllocatorUsedForAllocation(nullptr)
{

}

void* ShirosAllocator::Allocate(size_t bytes)
{
	if (bytes > m_maxAllowedObjectSize)
	{
		//our allocator can't handle object of this size
		//fallback on global default allocator
		return ::operator new(bytes);
	}

	//check if we already allocated an allocator, and if this one manage Chunk of the desired size
	if (m_lastAllocatorUsedForAllocation && m_lastAllocatorUsedForAllocation->GetBlockSize() == bytes)
	{
		return m_lastAllocatorUsedForAllocation->Allocate();
	}

	//find the correct allocator in list
	AllocatorPool::iterator it = std::lower_bound(m_Pool.begin(), m_Pool.end(), bytes, FixedAllocatorComparator);

	//if the allocator that manage this size_t is nowhere to be found then insert a new Allocator that'll do the work
	if (it == m_Pool.end() || it->GetBlockSize() != bytes)
	{
		it = m_Pool.insert(it, FixedAllocator(bytes));
		m_lastAllocatorUsedForDeallocation = &*m_Pool.begin();
	}
	
	//either we found an allocator or we're using the one we just inserted
	m_lastAllocatorUsedForAllocation = &*it;
	return m_lastAllocatorUsedForAllocation->Allocate();
}

void ShirosAllocator::Deallocate(void* p_obj, size_t size_obj)
{
	//if size is greater than the max allowed one fallback to global delete operator
	if (size_obj > m_maxAllowedObjectSize)
	{
		return ::operator delete(p_obj);
	}

	//check if the last time we deallocated something from an allocator, and if that allocator is of the desired size
	//if it is, then use that one
	if (m_lastAllocatorUsedForDeallocation && m_lastAllocatorUsedForDeallocation->GetBlockSize() == size_obj)
	{
		m_lastAllocatorUsedForDeallocation->Deallocate(p_obj);
		return;
	}
	//find the allocator used to allocate the object requested to release
	AllocatorPool::iterator it = std::lower_bound(m_Pool.begin(), m_Pool.end(), size_obj, FixedAllocatorComparator);
	//assert the allocator exists and it is of the right size
	//it MUST be impossible to delete an object that was previously allocated using our Allocator!
	assert(it != m_Pool.end());
	assert(it->GetBlockSize() == size_obj);
	m_lastAllocatorUsedForDeallocation = &*it;
	m_lastAllocatorUsedForDeallocation->Deallocate(p_obj);

}