#include "pch.h"
#include "ShirosMemoryManager.h"
#include <cassert>
ShirosMemoryManager& ShirosMemoryManager::Get()
{
	static ShirosMemoryManager mm;
	return mm;
}

size_t ShirosMemoryManager::CHUNK_SIZE = DEFAULT_CHUNK_SIZE;
size_t ShirosMemoryManager::MAX_SMALL_OBJ_SIZE = MAX_SMALL_OBJECT_SIZE;
size_t ShirosMemoryManager::FREE_LIST_SIZE = 16384;  // 16 KB
FreeListAllocator::FitPolicy ShirosMemoryManager::FREE_LIST_POLICY = FreeListAllocator::FitPolicy::BEST_FIT;

ShirosMemoryManager::ShirosMemoryManager()
	: m_smallObjAllocator(CHUNK_SIZE),
	m_freeListAllocator(FREE_LIST_SIZE, FREE_LIST_POLICY)
{

}

void* ShirosMemoryManager::Allocate(const size_t ObjSize, const size_t Alignment, char const* file, unsigned long line)
{
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in file " << file << endl;
	
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();
	void* p_res = nullptr;
	if (Instance.CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		p_res = Instance.m_smallObjAllocator.Allocate(ObjSize);
		cout << "Requested size is less or equal to MAX_SMALL_OBJECT_SIZE. Allocated memory using SmallObjAllocator" << endl;
	}
	else
	{
		p_res = Instance.m_freeListAllocator.Allocate(ObjSize, Alignment);
		cout << "Requested size is larger than MAX_SMALL_OBJECT_SIZE. Allocated memory using FreeListAllocator" << endl;
	}
	
	if (p_res)
	{
		cout << "Allocated memory starting from address " << p_res << endl;
		Instance.m_mem_used += ObjSize;
	}
	else
	{
		cout << "Allocation didn't complete correctly" << endl;
	}

	return p_res;
}

void ShirosMemoryManager::Deallocate(void* ptr,const size_t ObjSize, char const* file, unsigned long line)
{
	cout << "Requested deallocation of " << ObjSize << " bytes from address " << ptr << " requested by line " << line << " in file " << file << endl;
	
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();
	if (Instance.CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		Instance.m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else 
	{
		Instance.m_freeListAllocator.Deallocate(ptr);
	}

	Instance.m_mem_used -= ObjSize;
	Instance.m_mem_freed += ObjSize;
}

bool ShirosMemoryManager::CanBeHandledWithSmallObjAllocator(size_t ObjSize) const
{
	return ObjSize <= MAX_SMALL_OBJECT_SIZE;
}

void ShirosMemoryManager::PrintMemoryState()
{
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();

	size_t m_totAllocatedMemory = Instance.m_smallObjAllocator.GetTotalAllocatedMemory() + Instance.m_freeListAllocator.GetTotalAllocatedMemory(); //add also memory for general purpose allocator
	cout << "===== MEMORY STATE ======" << endl;
	cout << "| Total Memory Allocated: " << m_totAllocatedMemory << " |" << endl;
	cout << "| Memory Used: " << Instance.m_mem_used << " |" << endl;
	cout << "| Memory Freed: " << Instance.m_mem_freed << " |" << endl;
}
