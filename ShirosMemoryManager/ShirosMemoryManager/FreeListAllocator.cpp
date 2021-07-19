#include "pch.h"
#include "FreeListAllocator.h"

namespace {
	const std::size_t ComputePadding(const std::size_t InAddress, const std::size_t InAlignment)
	{
		std::size_t padding = InAlignment - (InAddress & InAlignment);

		return (padding == InAlignment)
			? 0 //aligned
			: padding;
	}

	const std::size_t ComputePaddingWithHeader(const std::size_t InAddress, const std::size_t InAlignment, const std::size_t InHeaderSize) {
		return InHeaderSize + ComputePadding(InAddress + InHeaderSize, InAlignment);
	}
}

FreeListAllocator::FreeListAllocator(const std::size_t TotalSize, const FitPolicy policy)
	: m_totalSizeAllocated(TotalSize), m_policy(policy)
{
	Reset();
}

FreeListAllocator::~FreeListAllocator()
{
	free(mp_start);
	mp_start = nullptr;
}

void FreeListAllocator::Reset()
{
	if (mp_start)
	{
		free(mp_start);
		mp_start = nullptr;
	}

	mp_start = malloc(m_totalSizeAllocated);

	Node* head = static_cast<Node*>(mp_start);
	head->data.blockSize = m_totalSizeAllocated;
	head->next = nullptr;
	m_freeList.head = nullptr;
	m_freeList.insert(nullptr, head);
}

void* FreeListAllocator::Allocate(const std::size_t AllocationSize, const std::size_t alignment)
{
	assert(AllocationSize > 0 && alignment > 0 && "Allocation Size and Alignment must be positive");
	
	static const std::size_t allocationHeaderSize = sizeof(FreeListAllocator::AllocatedBlockHeader);
	static const std::size_t freeHeaderSize = sizeof(FreeListAllocator::FreeBlockHeader);

	//Linearly search the list for a block that has enough space to allocate AllocationSize bytes
	std::size_t OutNewAddressPadding;
	Node* OutResultNode,* OutPrevNode;
	OutResultNode = OutPrevNode = nullptr;
	
	Find(AllocationSize, alignment, OutNewAddressPadding, OutPrevNode, OutResultNode);
	
	assert(OutResultNode && "Not enough Memory for any new Block");

	const std::size_t paddingToAlign = OutNewAddressPadding - allocationHeaderSize;
	const std::size_t sizeNeeded = AllocationSize + OutNewAddressPadding;

	const std::size_t remainingBlockSize = OutResultNode->data.blockSize - sizeNeeded;
	const std::size_t resNodeAddress = reinterpret_cast<std::size_t>(OutResultNode);

	if (remainingBlockSize > 0)
	{
		//split the node into a data block and a free block of size "remainingBlockSize"
		Node* freeNode = reinterpret_cast<Node*>(resNodeAddress + sizeNeeded);
		freeNode->data.blockSize = remainingBlockSize;
		m_freeList.insert(OutResultNode, freeNode);
	}

	m_freeList.remove(OutPrevNode, OutResultNode); //detach resultNode from freeList in order to use it

	//setup data for allocation block
	AllocatedBlockHeader* _header = reinterpret_cast<AllocatedBlockHeader*>(resNodeAddress + paddingToAlign);
	_header->blockSize = sizeNeeded;
	_header->padding = paddingToAlign;

	return _header;
}

void FreeListAllocator::Deallocate(void* ptr)
{
	const std::size_t address = reinterpret_cast<std::size_t>(ptr);
	const AllocatedBlockHeader* allocatedBlockHeader(reinterpret_cast<AllocatedBlockHeader*>(address));

	Node* freeNode = reinterpret_cast<Node*>(address);
	freeNode->data.blockSize = allocatedBlockHeader->blockSize + allocatedBlockHeader->padding; //take back allocated size
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
	//merge contiguous nodes
	Coalescence(prev, freeNode);
}

void FreeListAllocator::Coalescence(Node* prevBlock, Node* freeBlock)
{
	//TODO TEST
	if (freeBlock->next != nullptr && (reinterpret_cast<std::size_t>(freeBlock) + freeBlock->data.blockSize) == reinterpret_cast<std::size_t>(freeBlock->next))
	{
		freeBlock->data.blockSize += freeBlock->next->data.blockSize;
		m_freeList.remove(freeBlock, freeBlock->next);
	}

	if (prevBlock != nullptr && (reinterpret_cast<std::size_t>(prevBlock) + prevBlock->data.blockSize) == reinterpret_cast<std::size_t>(freeBlock))
	{
		prevBlock->data.blockSize += freeBlock->data.blockSize;
		m_freeList.remove(prevBlock, freeBlock);
	}
}

void FreeListAllocator::Find(const std::size_t InSize, const std::size_t InAlignment, std::size_t& OutPadding, Node*& OutPreviousNode, Node*& OutFoundNode)
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

void FreeListAllocator::FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& resNode)
{
	// Iterate the whole list and return a ptr with the best fit
	static const std::size_t smallestDiff = std::numeric_limits<std::size_t>::max();
	Node* bestBlock = nullptr;
	Node* it = m_freeList.head,
		* prev = nullptr;

	const std::size_t AllocationBlockHeaderAlignment = alignof(AllocatedBlockHeader);
	for (; it != nullptr; it = it->next)
	{
		const std::size_t InAlignment = AllocationBlockHeaderAlignment > alignment
			? AllocationBlockHeaderAlignment
			: alignment;
		padding = ComputePaddingWithHeader(reinterpret_cast<std::size_t>(it), InAlignment, sizeof(AllocatedBlockHeader));
		const std::size_t requiredSpace = size + padding;
		if (it->data.blockSize >= requiredSpace && ((it->data.blockSize - requiredSpace) < smallestDiff)) {
			bestBlock = it;
		}
		prev = it;
	}
	
	previousNode = prev == bestBlock ? nullptr : prev; //prevents assignment of the same block
	resNode = bestBlock;
}


void FreeListAllocator::FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& resNode)
{
	//just iterate list and return first node that can handle a new block of size "size"
	Node* it = m_freeList.head, * prev = nullptr;

	const std::size_t AllocationBlockHeaderAlignment = alignof(AllocatedBlockHeader);
	for (; it != nullptr; it = it->next)
	{
		const std::size_t InAlignment = AllocationBlockHeaderAlignment > alignment
			? AllocationBlockHeaderAlignment
			: alignment;
		padding = ComputePaddingWithHeader(reinterpret_cast<std::size_t>(it), InAlignment, sizeof(AllocatedBlockHeader));
		const std::size_t requiredSpace = size + padding;
		if (it->data.blockSize >= requiredSpace) {
			break; //this node can handle the required space
		}
		prev = it;
	}

	previousNode = prev;
	resNode = it;
}