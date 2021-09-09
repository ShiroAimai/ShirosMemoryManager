#include "pch.h"
#include "ShirosMemoryManager.h"

ShirosMMCreationParams ShirosMemoryManager::mmCreationParams = ShirosMMCreationParams();

ShirosMemoryManager& ShirosMemoryManager::Get()
{
	static ShirosMemoryManager mm;
	return mm;
}

void ShirosMemoryManager::Init(const ShirosMMCreationParams& params)
{
	mmCreationParams = params;
}

ShirosMemoryManager::ShirosMemoryManager()
	: m_smallObjAllocator(mmCreationParams.chunkSize),
	m_freeListAllocator(mmCreationParams.freeListMemoryPoolSize, mmCreationParams.freeListFitPolicy)
{

}

ShirosMemoryManager::~ShirosMemoryManager()
{
	cout << "===== RELEASED ALLOCATED MEMORY ======" << endl;
}

void* ShirosMemoryManager::Allocate(size_t ObjSize, AllocationType AllocType, size_t Alignment /* = alignof(std::max_align_t) */)
{	
	void* p_res = nullptr;

	size_t AllocationSize;
	if (CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		p_res = m_smallObjAllocator.Allocate(ObjSize, AllocationSize);
#ifdef MM_DEBUG
		cout << "Requested size is less or equal MAX_SMALL_OBJECT_SIZE(" << MAX_SMALL_OBJECT_SIZE << ")";
		cout << ". Allocated memory using SmallObjAllocator" << endl;
#endif
	}
	else
	{
		p_res = m_freeListAllocator.Allocate(ObjSize, Alignment, AllocationSize);
#ifdef MM_DEBUG
		cout << "Requested size is larger than MAX_SMALL_OBJECT_SIZE(" << MAX_SMALL_OBJECT_SIZE << ")";
		cout << ". Allocated memory using FreeListAllocator" << endl;
#endif
	}
	
	if (p_res && AllocationSize > 0)
	{
#ifdef MM_DEBUG
		cout << "Allocated " << AllocationSize << " bytes from address " << p_res << endl;
#endif
		
		if (AllocType == AllocationType::Collection)
		{
			m_arrayAllocationMap[p_res] = ObjSize;
		}

		m_mem_used += AllocationSize;
		m_mem_allocated += AllocationSize;
	}
	else
	{
		cout << "Allocation didn't complete correctly" << endl;
	}

	return p_res;
}

void ShirosMemoryManager::Deallocate(void* ptr, size_t ObjSize /* = 0 */)
{
	if (!ptr) //bad argument
	{
		cout << "Aborting deallocation. Bad argument: address: " << ptr << endl;
		return;
	}

	//if ObjSize is empty, check if ptr is key of internal array map 
	if (ObjSize == 0)
	{
		std::map<void*, size_t>::iterator it = m_arrayAllocationMap.find(ptr);
		if (it != m_arrayAllocationMap.end())
		{
			ObjSize = it->second;
			m_arrayAllocationMap.erase(it);
		}
		if (ObjSize == 0) //bad argument
		{
			cout << "Aborting deallocation. Bad argument size: " << ObjSize << endl;
			return;
		}
	}

	size_t DeallocatedSize = 0;
	if (CanBeHandledWithSmallObjAllocator(ObjSize))
	{
		DeallocatedSize = m_smallObjAllocator.Deallocate(ptr, ObjSize);
	}
	else 
	{
		DeallocatedSize = m_freeListAllocator.Deallocate(ptr);
	}
	
	assert(DeallocatedSize > 0 && "Deallocated size must be greater than zero");

#ifdef MM_DEBUG
	cout << "Deallocated " << DeallocatedSize << " bytes from address " << ptr << endl;
#endif

	m_mem_used -= DeallocatedSize;
	m_mem_freed += DeallocatedSize;
}

bool ShirosMemoryManager::CanBeHandledWithSmallObjAllocator(size_t ObjSize) const
{
	return ObjSize <= mmCreationParams.maxSizeForSmallObj;
}

void ShirosMemoryManager::PrintMemoryState()
{
	size_t m_totAllocatedMemory = m_smallObjAllocator.GetTotalAllocatedMemory() + m_freeListAllocator.GetTotalAllocatedMemory();
	cout << "===== MEMORY STATE ======" << endl;
	cout << "| Total Memory Allocated: " << m_totAllocatedMemory << " |" << endl;
	cout << "| Memory Allocated: " << m_mem_allocated << " |" << endl;
	cout << "| Memory Freed: " << m_mem_freed << " |" << endl;
	cout << "| Memory Currently used: " << m_mem_used << " |" << endl;
}

void ShirosMemoryManager::Reset()
{
	m_mem_allocated = 0;
	m_mem_freed = 0;
	m_mem_used = 0;
	m_freeListAllocator.Reset();
	m_smallObjAllocator.Reset();
}