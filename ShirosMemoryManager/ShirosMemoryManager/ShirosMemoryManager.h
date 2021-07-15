#pragma once
#include "ShirosSmallObjAllocator.h"

#define MM_NEW(SIZE) ShirosMemoryManager::Allocate(SIZE, __FILE__, __LINE__)
#define MM_DELETE(PTR, SIZE) ShirosMemoryManager::Deallocate(PTR, SIZE, __FILE__, __LINE__)

#define MM_NEW_A(LENGTH, SIZE) ShirosMemoryManager::Allocate(LENGTH * SIZE, __FILE__, __LINE__)
#define MM_DELETE_A(PTR, LENGTH, SIZE) ShirosMemoryManager::Deallocate(PTR, LENGTH * SIZE, __FILE__, __LINE__)

#define MM_MALLOC(SIZE) ShirosMemoryManager::Allocate(SIZE, __FILE__, __LINE__)
#define MM_FREE(PTR, SIZE) ShirosMemoryManager::Deallocate(PTR, SIZE, __FILE__, __LINE__)

class ShirosMemoryManager /*Singleton*/
{
public:
	ShirosMemoryManager(const ShirosMemoryManager&) = delete;
	ShirosMemoryManager& operator=(const ShirosMemoryManager&) = delete;

	static size_t CHUNK_SIZE;
	static size_t MAX_SMALL_OBJ_SIZE;

	static void* Allocate(size_t ObjSize, char const* file, unsigned long line);
	static void Deallocate(void* ptr, size_t ObjSize, char const* file, unsigned long line);
	
private:
	ShirosMemoryManager();
	bool CanBeHandledBySmallObjAllocator(size_t ObjSize) const;

	static ShirosMemoryManager& Get();

	ShirosSmallObjAllocator m_smallObjAllocator;
};

