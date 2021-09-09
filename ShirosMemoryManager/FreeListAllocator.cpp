#include "pch.h"
#include "FreeListAllocator.h"

namespace {
	/** Given an address it computes its padding taking into account the passed alignment */
	size_t ComputePadding(size_t InAddress, size_t InAlignment)
	{
		// if & bitwise operator returns 0 between current address and previous alignment, we are aligned
		size_t padding = InAlignment - (InAddress & (InAlignment - 1));

		return (padding == InAlignment)
			? 0 //aligned
			: padding;
	}

	/** Given an address and an header size, it computes its padding taking into account the passed alignment */
	size_t ComputePaddingWithHeader(size_t InAddress, size_t InAlignment, size_t InHeaderSize) {
		return InHeaderSize + ComputePadding(InAddress + InHeaderSize, InAlignment);
	}

	/** Check if the given address is aligned to the alignment passed */
	bool isAligned(size_t address, size_t alignment)
	{
		return address % alignment== 0;
	}

}

FreeListAllocator::FreeListAllocator(size_t TotalSize, FitPolicy policy)
	: m_totalSizeAllocated(TotalSize), m_policy(policy)
{
	Reset();
}

void FreeListAllocator::Release()
{
	free(mp_start);
	mp_start = nullptr;
}

FreeListAllocator::~FreeListAllocator()
{
	Release();
}

void FreeListAllocator::Reset()
{
	if (mp_start)
	{
		Release();
	}

	mp_start = malloc(m_totalSizeAllocated);

	//interpret allocated memory as a unique big Node
	Node* head = static_cast<Node*>(mp_start);
	
	assert(head != nullptr);

	head->data.blockSize = m_totalSizeAllocated;
	head->next = nullptr;
	m_freeList.head = nullptr;
	m_freeList.insert(nullptr, head);
}

void* FreeListAllocator::Allocate(size_t AllocationSize, size_t alignment, size_t& OutAllocationSize)
{
	assert(AllocationSize > 0 && alignment > 0 && "Allocation Size and Alignment must be positive");
	
	static constexpr size_t allocationHeaderSize = sizeof(FreeListAllocator::AllocatedBlockHeader);

	//Linearly search the list for a block that has enough space to allocate AllocationSize bytes
	size_t OutNewAddressPadding;
	Node* OutResultNode,* OutPrevNode;
	OutResultNode = OutPrevNode = nullptr;
	
	Find(AllocationSize, alignment, OutNewAddressPadding, OutPrevNode, OutResultNode);
	
	assert(OutResultNode && "Not enough Memory for any new Block");

	//OutNewAddressPadding contains both 
	//subtract AllocatedBlockHeader size to get just the alignment padding
	const size_t alignmentPadding = OutNewAddressPadding - allocationHeaderSize; 
	//required size is (RequestedAllocationSize + Padding)
	const size_t requiredSize = AllocationSize + OutNewAddressPadding;

	const size_t remainingBlockSize = OutResultNode->data.blockSize - requiredSize;
	const size_t resNodeAddress = reinterpret_cast<size_t>(OutResultNode);

	if (remainingBlockSize > 0)
	{
		//add new free block of size "remainingBlockSize" just after the result node
		Node* freeNode = reinterpret_cast<Node*>(resNodeAddress + requiredSize);
		freeNode->data.blockSize = remainingBlockSize;
		m_freeList.insert(OutResultNode, freeNode);
	}

	m_freeList.remove(OutPrevNode, OutResultNode); //detach resultNode from freeList in order to use it

	//setup data for allocation block
	const size_t AllocatedBlockHeaderAddress = resNodeAddress + alignmentPadding;
	const size_t dataAddress = AllocatedBlockHeaderAddress + allocationHeaderSize;
	AllocatedBlockHeader* _header = reinterpret_cast<AllocatedBlockHeader*>(AllocatedBlockHeaderAddress);
	_header->blockSize = requiredSize;
	_header->padding = static_cast<char>(alignmentPadding);

	assert(isAligned(dataAddress, alignment));

	OutAllocationSize = requiredSize;
	return reinterpret_cast<void*>(dataAddress);
}

size_t FreeListAllocator::Deallocate(void* ptr)
{
	static constexpr size_t AllocationHeaderSize = sizeof(AllocatedBlockHeader);
	
	const size_t address = reinterpret_cast<size_t>(ptr);
	//get back allocationHeaderSize from ptr subtracting header block size
	const size_t allocatedBlockHeaderAddress = address - AllocationHeaderSize;
	//reinterpret the obtained address as a pointer to AllocationBlockHeader to access its members
	const AllocatedBlockHeader* allocatedBlockHeader = reinterpret_cast<AllocatedBlockHeader*>(allocatedBlockHeaderAddress);

	assert(allocatedBlockHeader != nullptr);
	assert(allocatedBlockHeader->blockSize >= AllocationHeaderSize);

	const size_t AlignmentPadding = static_cast<size_t>(allocatedBlockHeader->padding);
	const size_t DeallocationSize = allocatedBlockHeader->blockSize;

	//retrieve base address subtracting alignmentPadding e allocation header size from ptr
	Node* freeNode = reinterpret_cast<Node*>(allocatedBlockHeaderAddress - AlignmentPadding);
	freeNode->data.blockSize = DeallocationSize; //take back allocated size
	freeNode->next = nullptr;

	//iterate and insert the new free node back in the correct position inside the free list
	Node* it = m_freeList.head, * prev = nullptr;
	for (; it != nullptr; it = it->next) {
		if (ptr < it) {
			m_freeList.insert(prev, freeNode); //found the correct position inside the list
			break;
		}
		prev = it;
	}
	//try to merge contiguous nodes into a unique free block
	Coalescence(prev, freeNode);

	return DeallocationSize;
}
 
void FreeListAllocator::Coalescence(Node* prevBlock, Node* freeBlock)
{
	if (freeBlock->next != nullptr && (reinterpret_cast<size_t>(freeBlock) + freeBlock->data.blockSize) == reinterpret_cast<size_t>(freeBlock->next))
	{
		freeBlock->data.blockSize += freeBlock->next->data.blockSize;
		m_freeList.remove(freeBlock, freeBlock->next);
	}

	if (prevBlock != nullptr && (reinterpret_cast<size_t>(prevBlock) + prevBlock->data.blockSize) == reinterpret_cast<size_t>(freeBlock))
	{
		prevBlock->data.blockSize += freeBlock->data.blockSize;
		m_freeList.remove(prevBlock, freeBlock);
	}
}

void FreeListAllocator::Find(size_t InSize, size_t InAlignment, size_t& OutPadding, Node*& OutPreviousNode, Node*& OutFoundNode)
{
	switch (m_policy)
	{
	case FitPolicy::BEST_FIT:
		FindBest(InSize, InAlignment, OutPadding, OutPreviousNode, OutFoundNode);
		break; 
	case FitPolicy::FIRST_FIT:
		FindFirst(InSize, InAlignment, OutPadding, OutPreviousNode, OutFoundNode);
		break;
	}
}

void FreeListAllocator::FindBest(size_t size, size_t alignment, size_t& padding, Node*& previousNode, Node*& resNode)
{
	// Iterate the whole list and return a ptr with the best fit
	
	Node* bestBlock = nullptr, *prevBest = nullptr;
	Node* it = m_freeList.head,
		* prev = nullptr;
	/** Current smallest difference (blockSize - requiredSize) among all FreeBlock blocks*/
	size_t smallestDiff = std::numeric_limits<size_t>::max();
	for (; it != nullptr; it = it->next)
	{
		padding = ComputePaddingWithHeader(reinterpret_cast<size_t>(it), alignment, sizeof(AllocatedBlockHeader));
		const size_t requiredSpace = size + padding;
		if (it->data.blockSize >= requiredSpace && ((it->data.blockSize - requiredSpace) < smallestDiff)) {
			smallestDiff = it->data.blockSize - requiredSpace;
			bestBlock = it;
			prevBest = prev;
		}
		prev = it;
	}
	
	previousNode = prevBest;
	resNode = bestBlock;
}

void FreeListAllocator::FindFirst(size_t size, size_t alignment, size_t& padding, Node*& previousNode, Node*& resNode)
{
	//just iterate list and return first node that can handle a new block of size "size"
	Node* it = m_freeList.head, * prev = nullptr;

	for (; it != nullptr; it = it->next)
	{
		padding = ComputePaddingWithHeader(reinterpret_cast<size_t>(it), alignment, sizeof(AllocatedBlockHeader));
		const size_t requiredSpace = size + padding;
		if (it->data.blockSize >= requiredSpace) {
			break; //this node can handle the required space
		}
		prev = it;
	}

	previousNode = prev;
	resNode = it;
}