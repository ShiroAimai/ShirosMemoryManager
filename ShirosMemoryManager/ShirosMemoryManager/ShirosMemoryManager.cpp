#include "pch.h"
#include "ShirosMemoryManager.h"

ShirosMemoryManager& ShirosMemoryManager::Get()
{
	static ShirosMemoryManager mm;
	return mm;
}

size_t ShirosMemoryManager::CHUNK_SIZE = DEFAULT_CHUNK_SIZE;
size_t ShirosMemoryManager::MAX_SMALL_OBJ_SIZE = MAX_SMALL_OBJECT_SIZE;

ShirosMemoryManager::ShirosMemoryManager()
	: m_smallObjAllocator(CHUNK_SIZE)
{

}

void* ShirosMemoryManager::Allocate(size_t ObjSize, char const* file, unsigned long line)
{
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in file " << file << endl;
	
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();
	void* p_res = nullptr;
	if (Instance.CanBeHandledBySmallObjAllocator(ObjSize))
	{
		p_res = Instance.m_smallObjAllocator.Allocate(ObjSize);
	}
	else
	{
		//todo general purpose allocation
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

void ShirosMemoryManager::Deallocate(void* ptr, size_t ObjSize, char const* file, unsigned long line)
{
	cout << "Requested deallocation of " << ObjSize << " bytes from address " << ptr << " requested by line " << line << " in file " << file << endl;
	
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();
	if (Instance.CanBeHandledBySmallObjAllocator(ObjSize))
	{
		Instance.m_smallObjAllocator.Deallocate(ptr, ObjSize);
		Instance.m_mem_used -= ObjSize;
		Instance.m_mem_freed += ObjSize;
	}
	else 
	{
		//todo general purpose deallocation
	}
}

bool ShirosMemoryManager::CanBeHandledBySmallObjAllocator(size_t ObjSize) const
{
	return ObjSize <= MAX_SMALL_OBJECT_SIZE;
}

void ShirosMemoryManager::PrintMemoryState()
{
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();

	size_t m_totAllocatedMemory = Instance.m_smallObjAllocator.GetTotalAllocatedMemory(); //add also memory for general purpose allocator
	cout << "===== MEMORY STATE ======" << endl;
	cout << "| Total Memory Allocated: " << m_totAllocatedMemory << " |" << endl;
	cout << "| Memory Used: " << Instance.m_mem_used << " |" << endl;
	cout << "| Memory Freed: " << Instance.m_mem_freed << " |" << endl;
}
