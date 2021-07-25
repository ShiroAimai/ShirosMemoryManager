#pragma once
#include "SmallObjAllocator.h"
#include "FreeListAllocator.h"
#include "Mallocator.h"
#include <iostream>
#include <map>

using std::cout;
using std::endl;

class ShirosMemoryManager /*Singleton*/
{
public:
	enum class AllocationType
	{
		Single,
		Collection
	};

	ShirosMemoryManager(const ShirosMemoryManager&) = delete;
	ShirosMemoryManager& operator=(const ShirosMemoryManager&) = delete;

	static size_t CHUNK_SIZE;
	static size_t MAX_SMALL_OBJ_SIZE;
	static size_t FREE_LIST_SIZE;
	static FreeListAllocator::FitPolicy FREE_LIST_POLICY;

	static ShirosMemoryManager& Get();

	void* Allocate(const size_t ObjSize, const AllocationType AllocType, const size_t Alignment = alignof(std::max_align_t));
	void Deallocate(void* ptr, size_t ObjSize = 0);
	
	void Reset();
	void PrintMemoryState();

	inline const size_t GetCurrentlyUsedMemory() { return m_mem_used; }
	inline const size_t GetMemoryRequested() { return m_mem_requested; }
	inline const size_t GetMemoryFreed() { return m_mem_freed; }
private:
	ShirosMemoryManager();

	bool CanBeHandledWithSmallObjAllocator(size_t ObjSize) const;

	size_t m_mem_used = 0;
	size_t m_mem_requested = 0;
	size_t m_mem_freed = 0;

	SmallObjAllocator m_smallObjAllocator;
	FreeListAllocator m_freeListAllocator;

	std::map<void*, size_t, std::less<void*>, Mallocator<std::pair<void*, size_t>>> m_arrayAllocationMap;
};


inline void* operator new(size_t ObjSize, size_t Alignment, char const* function, char const* file, unsigned long line)
{
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in function " << function << " in file " << file << endl;
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Single, Alignment);
}

inline void operator delete(void* ptr, size_t ObjSize, char const* function, char const* file, unsigned long line) noexcept
{
	cout << "Requested deallocation of address " << ptr << " requested by line " << line << " in function " << function << " in file " << file << endl;
	ShirosMemoryManager::Get().Deallocate(ptr, ObjSize);
}

template <typename T>
inline void _Delete(T* ptr, size_t Size, char const* function, char const* file, unsigned long line)
{
	if (ptr)
	{
		ptr->~T();
		operator delete(static_cast<void*>(ptr), Size, function, file, line);
	}
}
template<>
inline void _Delete<void>(void* ptr, size_t Size, char const* function, char const* file, unsigned long line)
{
	if (ptr)
	{
		operator delete(ptr, Size, function, file, line);
	}
}

inline void* operator new[](size_t ObjSize, size_t Alignment, char const* function, char const* file, unsigned long line)
{
	cout << "Requested allocation of array of " << ObjSize << " bytes requested by line " << line << " in function " << function << " in file " << file << endl;
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Collection, Alignment);
}

inline void operator delete[](void* ptr, size_t Length, char const* function, char const* file, unsigned long line)
{
	cout << "Requested deallocation of array of " << Length << " elements from address " << ptr << " requested by line " << line << " in function " << function << " in file " << file << endl;
	ShirosMemoryManager::Get().Deallocate(ptr); //we do not pass size here because it is already saved inside the internal array allocation map inside the MM
}

template <typename T>
inline void _DeleteArr(T* ptr, size_t Length, char const* function, char const* file, unsigned long line)
{
	if (ptr)
	{
		for (int i = 0; i < Length; ++i)
		{
			(ptr + i)->~T(); //destroy all array elements
		}
		operator delete[](static_cast<void*>(ptr), Length, function, file, line);
	}
}

template<>
inline void _DeleteArr<void>(void* ptr, size_t Length, char const* function, char const* file, unsigned long line)
{
	if (ptr)
	{
		operator delete[](ptr, Length, function, file, line);
	}
}

inline void* _Malloc(size_t ObjSize, char const* function, char const* file, unsigned long line)
{
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in function " << function << " in file " << file << endl;
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Single, alignof(std::max_align_t));
}

inline void _Free(void* ptr, size_t ObjSize, char const* function, char const* file, unsigned long line)
{
	cout << "Requested deallocation of " << ObjSize << " bytes from address " << ptr << " requested by line " << line << " in function " << function << " in file " << file << endl;
	ShirosMemoryManager::Get().Deallocate(ptr, ObjSize);
}

#define MM_NEW(ALIGNMENT) new(ALIGNMENT, __FUNCTION__, __FILE__, __LINE__)
#define MM_DELETE(PTR, SIZE) _Delete(PTR, SIZE, __FUNCTION__, __FILE__, __LINE__)

#define MM_NEW_A(T, LENGTH) new(alignof(T), __FUNCTION__, __FILE__, __LINE__) T[LENGTH]
#define MM_DELETE_A(PTR, LENGTH) _DeleteArr(PTR, LENGTH, __FUNCTION__, __FILE__, __LINE__)

#define MM_MALLOC(SIZE) _Malloc(SIZE, __FUNCTION__, __FILE__, __LINE__)
#define MM_FREE(PTR, SIZE) _Free(PTR, SIZE, __FUNCTION__, __FILE__, __LINE__)
