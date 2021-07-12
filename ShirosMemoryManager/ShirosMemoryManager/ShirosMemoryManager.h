#pragma once
#include "ShirosSmallObjAllocator.h"

class ShirosMemoryManager
{
public:
	ShirosMemoryManager(size_t ChunkSize, size_t MaxSize);

	ShirosMemoryManager(const ShirosMemoryManager&) = delete;
	ShirosMemoryManager& operator=(const ShirosMemoryManager&) = delete;

	void* MM_NEW(size_t ObjSize);
	void* MM_NEW_A(size_t ObjSize);

	void MM_DELETE(void* ptr, size_t ObjSize);
	void MM_DELETE_A(void* ptr, size_t ObjSize);

	void* MM_MALLOC(size_t ObjSize);
	void MM_FREE(void* ptr, size_t ObjSize);
private:
	size_t m_maxAllowedObjectSize;

	ShirosSmallObjAllocator m_smallObjAllocator;

	bool CanBeHandledBySmallObjAllocator(size_t ObjSize) const;
};

