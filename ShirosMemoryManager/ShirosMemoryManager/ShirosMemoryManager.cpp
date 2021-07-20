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
size_t ShirosMemoryManager::FREE_LIST_SIZE = 65536;  // 64 KB
FreeListAllocator::FitPolicy ShirosMemoryManager::FREE_LIST_POLICY = FreeListAllocator::FitPolicy::BEST_FIT;

ShirosMemoryManager::ShirosMemoryManager()
	: m_smallObjAllocator(CHUNK_SIZE),
	m_freeListAllocator(FREE_LIST_SIZE, FREE_LIST_POLICY)
{

}

void* ShirosMemoryManager::Allocate(const size_t ObjSize, const size_t Alignment, char const* file, unsigned long line)
{
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in file " << file << endl;
	
	void* p_res = nullptr;

	if (CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		p_res = m_smallObjAllocator.Allocate(ObjSize);
		cout << "Requested size is less or equal to MAX_SMALL_OBJECT_SIZE. Allocated memory using SmallObjAllocator" << endl;
	}
	else
	{
		p_res = m_freeListAllocator.Allocate(ObjSize, Alignment);
		cout << "Requested size is larger than MAX_SMALL_OBJECT_SIZE. Allocated memory using FreeListAllocator" << endl;
	}
	
	if (p_res)
	{
		cout << "Allocated memory starting from address " << p_res << endl;
		m_mem_used += ObjSize;
		m_mem_requested += ObjSize;
	}
	else
	{
		cout << "Allocation didn't complete correctly" << endl;
	}

	return p_res;
}

void ShirosMemoryManager::Deallocate(void*& ptr,const size_t ObjSize, char const* file, unsigned long line)
{
	if (!ptr || ObjSize == 0) return; //bad arguments

	cout << "Requested deallocation of " << ObjSize << " bytes from address " << ptr << " requested by line " << line << " in file " << file << endl;
	
	if (CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else 
	{
		m_freeListAllocator.Deallocate(ptr);
	}

	m_mem_used -= ObjSize;
	m_mem_freed += ObjSize;

	ptr = nullptr; //prevents repeated calls with this ptr
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
