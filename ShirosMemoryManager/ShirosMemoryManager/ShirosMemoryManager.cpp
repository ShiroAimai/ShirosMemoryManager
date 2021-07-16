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
	if (Instance.CanBeHandledBySmallObjAllocator(ObjSize))
	{
		return Instance.m_smallObjAllocator.Allocate(ObjSize);
	}
	else
	{
		//todo general purpose allocation
		return nullptr;
	}
}

void ShirosMemoryManager::Deallocate(void* ptr, size_t ObjSize, char const* file, unsigned long line)
{
	cout << "Requested deallocation of " << ObjSize << " bytes from address " << ptr << " requested by line " << line << " in file " << file << endl;
	
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();
	if (Instance.CanBeHandledBySmallObjAllocator(ObjSize))
	{
		Instance.m_smallObjAllocator.Deallocate(ptr, ObjSize);
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