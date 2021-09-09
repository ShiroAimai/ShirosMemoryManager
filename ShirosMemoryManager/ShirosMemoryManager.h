#pragma once
#include "SmallObjAllocator.h"
#include "FreeListAllocator.h"
#include "Mallocator.h"
#include <iostream>
#include <map>

using std::cout;
using std::endl;

#ifdef _DEBUG
#define MM_DEBUG
#endif

struct ShirosMMCreationParams
{
	/** Chunk size for SmallObjectAllocator. Default is 4Kb */
	size_t chunkSize = DEFAULT_CHUNK_SIZE;
	/** Max size manageable by SmallObjAllocator. Default is 128 bytes */
	size_t maxSizeForSmallObj = MAX_SMALL_OBJECT_SIZE;
	/** Memory pool to preallocate for FreeListAllocator. Default is 1MB */
	size_t freeListMemoryPoolSize = 67108864;  // 64 MB
	/** Fit policy to use for FreeListAllocator. Default is BestFit*/
	FreeListAllocator::FitPolicy freeListFitPolicy = FreeListAllocator::FitPolicy::BEST_FIT;
};

class ShirosMemoryManager /*Singleton*/
{
public:
	enum class AllocationType
	{
		Single,
		Collection
	};
	
	/** Use this initialization function to override Memory Manager default creation parameters */
	/** BE CAREFUL: Values is not checked, its up to the client to give consistent values */
	static void Init(const ShirosMMCreationParams& params);
	static ShirosMemoryManager& Get();

	~ShirosMemoryManager();
	/** Prevent copy for this class */
	ShirosMemoryManager(const ShirosMemoryManager&) = delete;
	ShirosMemoryManager& operator=(const ShirosMemoryManager&) = delete;

	void* Allocate(size_t ObjSize, AllocationType AllocType, size_t Alignment = alignof(std::max_align_t));
	void Deallocate(void* ptr, size_t ObjSize = 0);
	
	void Reset();
	void PrintMemoryState();

	inline const size_t GetCurrentlyUsedMemory() { return m_mem_used; }
	inline const size_t GetMemoryRequested() { return m_mem_allocated; }
	inline const size_t GetMemoryFreed() { return m_mem_freed; }
private:
	ShirosMemoryManager();
	static ShirosMMCreationParams mmCreationParams;

	bool CanBeHandledWithSmallObjAllocator(size_t ObjSize) const;

	size_t m_mem_used = 0;
	size_t m_mem_allocated = 0;
	size_t m_mem_freed = 0;

	//TODO : Refactor allocator instances. Maybe making a common allocator interface?
	/** Allocator for SmallObjects */
	SmallObjAllocator m_smallObjAllocator;
	/** Allocator for LargeObjects. Large objects are identified by a size_t > MAX_SMALL_OBJ_SIZE*/
	FreeListAllocator m_freeListAllocator;

	/** 
	 *  Internal hash map. It is used to track every AllocationType::Collection request. 
	 *  Its goal is to store the allocated size as a map entry, using its memory address as key.
	 */
	std::map<void*, size_t, std::less<void*>, Mallocator<std::pair<void*, size_t>>> m_arrayAllocationMap;
};

inline void* operator new(size_t ObjSize, size_t Alignment, char const* function, char const* file, unsigned long line)
{
#ifdef MM_DEBUG
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in function " << function << " in file " << file << endl;
#endif

	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Single, Alignment);
}

inline void operator delete(void* ptr, size_t ObjSize, char const* function, char const* file, unsigned long line) noexcept
{
#ifdef MM_DEBUG
	cout << "Requested deallocation of address " << ptr << " requested by line " << line << " in function " << function << " in file " << file << endl;
#endif
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
#ifdef MM_DEBUG
	cout << "Requested allocation of array of " << ObjSize << " bytes requested by line " << line << " in function " << function << " in file " << file << endl;
#endif
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Collection, Alignment);
}

inline void operator delete[](void* ptr, size_t Length, char const* function, char const* file, unsigned long line)
{
#ifdef MM_DEBUG
	cout << "Requested deallocation of array of " << Length << " elements from address " << ptr << " requested by line " << line << " in function " << function << " in file " << file << endl;
#endif
	ShirosMemoryManager::Get().Deallocate(ptr); //we do not pass size here because it is already saved inside the internal array allocation map inside the MM
}

template <typename T>
inline void _DeleteArr(T* ptr, size_t Length, char const* function, char const* file, unsigned long line)
{
	if (ptr)
	{
		for (unsigned int i = 0; i < Length; ++i)
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
#ifdef MM_DEBUG
	cout << "Requested allocation of " << ObjSize << " bytes requested by line " << line << " in function " << function << " in file " << file << endl;
#endif
	return ShirosMemoryManager::Get().Allocate(ObjSize, ShirosMemoryManager::AllocationType::Single, alignof(std::max_align_t));
}

inline void _Free(void* ptr, size_t ObjSize, char const* function, char const* file, unsigned long line)
{
#ifdef MM_DEBUG
	cout << "Requested deallocation of " << ObjSize << " bytes from address " << ptr << " requested by line " << line << " in function " << function << " in file " << file << endl;
#endif
	ShirosMemoryManager::Get().Deallocate(ptr, ObjSize);
}

#define MM_NEW(ALIGNMENT) new(ALIGNMENT, __FUNCTION__, __FILE__, __LINE__)
#define MM_DELETE(PTR, SIZE) _Delete(PTR, SIZE, __FUNCTION__, __FILE__, __LINE__)

#define MM_NEW_A(T, LENGTH) new(alignof(T), __FUNCTION__, __FILE__, __LINE__) T[LENGTH]
#define MM_DELETE_A(PTR, LENGTH) _DeleteArr(PTR, LENGTH, __FUNCTION__, __FILE__, __LINE__)

#define MM_MALLOC(SIZE) _Malloc(SIZE, __FUNCTION__, __FILE__, __LINE__)
#define MM_FREE(PTR, SIZE) _Free(PTR, SIZE, __FUNCTION__, __FILE__, __LINE__)
