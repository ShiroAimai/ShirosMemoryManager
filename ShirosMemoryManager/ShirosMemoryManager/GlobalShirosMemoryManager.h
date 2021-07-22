#pragma once
#include <cstdlib>
#include "ShirosMemoryManager.h"

#ifdef GLOBAL_SHIRO_MM

void* operator new(std::size_t ObjSize)
{
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Single);
}

void* operator new[](size_t ObjSize)
{
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Collection);
}

void operator delete(void* ptr, size_t ObjSize) noexcept
{
	ShirosMemoryManager::Get().Deallocate(ptr, ObjSize);
}

void operator delete[](void* ptr) noexcept
{
	ShirosMemoryManager::Get().Deallocate(ptr);
}

#endif