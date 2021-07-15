#pragma once
#include <cstdlib>
#include "ShirosMemoryManager.h"

#ifdef GLOBAL_SHIRO_MM

void* operator new(size_t ObjSize)
{
	return MM_NEW(ObjSize);
}

void* operator new[](size_t ArraySize, size_t ObjSize)
{
	return MM_NEW_A(ArraySize, ObjSize);
}

void operator delete(void* ptr, size_t ObjSize)
{
	MM_DELETE(ptr, ObjSize);
}

void operator delete[](void* ptr, size_t ArraySize, size_t ObjSize)
{
	MM_DELETE_A(ptr, ArraySize, ObjSize);
}

#endif