#include "pch.h"
#include "ShirosMemoryManager.h"

ShirosMemoryManager& ShirosMemoryManager::Get()
{
	static ShirosMemoryManager mm;
	return mm;
}

size_t ShirosMemoryManager::CHUNK_SIZE = DEFAULT_CHUNK_SIZE;
size_t ShirosMemoryManager::MAX_SMALL_OBJ_SIZE = MAX_SMALL_OBJECT_SIZE;
size_t ShirosMemoryManager::FREE_LIST_SIZE = 65536;  // 64 KB
FreeListAllocator::FitPolicy ShirosMemoryManager::FREE_LIST_POLICY = FreeListAllocator::FitPolicy::BEST_FIT;

ShirosMemoryManager::ShirosMemoryManager()
	: m_smallObjAllocator(CHUNK_SIZE),
	m_freeListAllocator(FREE_LIST_SIZE, FREE_LIST_POLICY)
{

}

void* ShirosMemoryManager::Allocate(const size_t ObjSize, const AllocationType AllocType, const size_t Alignment /* = alignof(std::max_align_t) */)
{	
	void* p_res = nullptr;

	if (CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		p_res = m_smallObjAllocator.Allocate(ObjSize);
		cout << "Requested size is less or equal MAX_SMALL_OBJECT_SIZE(" << MAX_SMALL_OBJECT_SIZE << ")";
		cout << ". Allocated memory using SmallObjAllocator" << endl;
	}
	else
	{
		p_res = m_freeListAllocator.Allocate(ObjSize, Alignment);
		cout << "Requested size is larger than MAX_SMALL_OBJECT_SIZE(" << MAX_SMALL_OBJECT_SIZE << ")";
		cout << ". Allocated memory using FreeListAllocator" << endl;
	}
	
	if (p_res)
	{
		cout << "Allocated " << ObjSize << " bytes from address " << p_res << endl;
		
		if (AllocType == AllocationType::Collection)
		{
			m_arrayAllocationMap[p_res] = ObjSize;
		}

		m_mem_used += ObjSize;
		m_mem_requested += ObjSize;
	}
	else
	{
		cout << "Allocation didn't complete correctly" << endl;
	}

	return p_res;
}

void ShirosMemoryManager::Deallocate(void* ptr, size_t ObjSize /* = 0 */)
{
	if (!ptr) //bad argument
	{
		cout << "Aborting deallocation. Bad argument: address: " << ptr << endl;
	}

	//if ObjSize is empty, check if ptr is key of internal array map 
	if (ObjSize == 0)
	{
		std::map<void*, size_t>::iterator it = m_arrayAllocationMap.find(ptr);
		if (it != m_arrayAllocationMap.end())
		{
			ObjSize = it->second;
			m_arrayAllocationMap.erase(it);
		}
		if (ObjSize == 0) //bad argument
		{
			cout << "Aborting deallocation. Bad argument size: " << ObjSize << endl;
			return;
		}
	}

	if (CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else 
	{
		m_freeListAllocator.Deallocate(ptr);
	}

	cout << "Deallocated " << ObjSize << " bytes from address " << ptr << endl;

	m_mem_used -= ObjSize;
	m_mem_freed += ObjSize;
}

bool ShirosMemoryManager::CanBeHandledWithSmallObjAllocator(size_t ObjSize) const
{
	return ObjSize <= MAX_SMALL_OBJECT_SIZE;
}

void ShirosMemoryManager::PrintMemoryState()
{

	size_t m_totAllocatedMemory = m_smallObjAllocator.GetTotalAllocatedMemory() + m_freeListAllocator.GetTotalAllocatedMemory();
	cout << "===== MEMORY STATE ======" << endl;
	cout << "| Total Memory Allocated: " << m_totAllocatedMemory << " |" << endl;
	cout << "| Memory Requested: " << m_mem_requested << " |" << endl;
	cout << "| Memory Freed: " << m_mem_freed << " |" << endl;
	cout << "| Memory Currently used: " << m_mem_used << " |" << endl;
}

void ShirosMemoryManager::Reset()
{
	m_mem_requested = 0;
	m_mem_freed = 0;
	m_mem_used = 0;
	m_freeListAllocator.Reset();
	m_smallObjAllocator.Reset();
}
