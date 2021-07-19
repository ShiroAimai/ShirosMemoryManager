#pragma once
#include "ShirosSmallObjAllocator.h"
#include "FreeListAllocator.h"

#define MM_NEW(SIZE, ALIGNMENT) ShirosMemoryManager::Allocate(SIZE, ALIGNMENT, __FILE__, __LINE__)
#define MM_DELETE(PTR, SIZE) ShirosMemoryManager::Deallocate(PTR, SIZE, __FILE__, __LINE__)

#define MM_NEW_A(LENGTH, SIZE, ALIGNMENT) ShirosMemoryManager::Allocate(LENGTH * SIZE, ALIGNMENT, __FILE__, __LINE__)
#define MM_DELETE_A(PTR, LENGTH, SIZE) ShirosMemoryManager::Deallocate(PTR, LENGTH * SIZE, __FILE__, __LINE__)

#define MM_MALLOC(SIZE, ALIGNMENT) ShirosMemoryManager::Allocate(SIZE, ALIGNMENT, __FILE__, __LINE__)
#define MM_FREE(PTR, SIZE) ShirosMemoryManager::Deallocate(PTR, SIZE, __FILE__, __LINE__)

class ShirosMemoryManager /*Singleton*/
{
public:
	ShirosMemoryManager(const ShirosMemoryManager&) = delete;
	ShirosMemoryManager& operator=(const ShirosMemoryManager&) = delete;

	static size_t CHUNK_SIZE;
	static size_t MAX_SMALL_OBJ_SIZE;
	static size_t FREE_LIST_SIZE;
	static FreeListAllocator::FitPolicy FREE_LIST_POLICY;

	static void* Allocate(const size_t ObjSize, const size_t Alignment, char const* file, unsigned long line);
	static void Deallocate(void* ptr, const size_t ObjSize, char const* file, unsigned long line);
	
	static void PrintMemoryState();
private:
	ShirosMemoryManager();
	bool CanBeHandledWithSmallObjAllocator(size_t ObjSize) const;

	static ShirosMemoryManager& Get();

	size_t m_mem_used = 0;
	size_t m_mem_freed = 0;

	ShirosSmallObjAllocator m_smallObjAllocator;
	FreeListAllocator m_freeListAllocator;
};

