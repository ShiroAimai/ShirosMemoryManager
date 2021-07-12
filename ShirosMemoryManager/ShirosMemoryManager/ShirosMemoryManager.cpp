#include "pch.h"
#include "ShirosMemoryManager.h"

ShirosMemoryManager::ShirosMemoryManager(size_t ChunkSize, size_t MaxSize)
	: m_maxAllowedObjectSize(MaxSize),
	m_smallObjAllocator(ChunkSize)
{

}

bool ShirosMemoryManager::CanBeHandledBySmallObjAllocator(size_t ObjSize) const
{
	return ObjSize <= MAX_SMALL_OBJECT_SIZE;
}


void* ShirosMemoryManager::MM_NEW(size_t ObjSize)
{
	if (CanBeHandledBySmallObjAllocator(ObjSize))
	{
		return m_smallObjAllocator.Allocate(ObjSize);
	}
	else 
	{
		return ::operator new(ObjSize);
	}
}

void* ShirosMemoryManager::MM_NEW_A(size_t ObjSize)
{
	if (CanBeHandledBySmallObjAllocator(ObjSize))
	{
		//m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else
	{
		return ::operator new[](ObjSize);
	}
}

void ShirosMemoryManager::MM_DELETE(void* ptr, size_t ObjSize) 
{
	if (CanBeHandledBySmallObjAllocator(ObjSize))
	{
		m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else 
	{
		::operator delete(ptr);
	}
}

void ShirosMemoryManager::MM_DELETE_A(void* ptr, size_t ObjSize)
{
	if (CanBeHandledBySmallObjAllocator(ObjSize))
	{
		//m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else
	{
		::operator delete[](ptr);
	}
}

void* ShirosMemoryManager::MM_MALLOC(size_t ObjSize) 
{

}

void ShirosMemoryManager::MM_FREE(void* ptr, size_t ObjSize)
{

}
