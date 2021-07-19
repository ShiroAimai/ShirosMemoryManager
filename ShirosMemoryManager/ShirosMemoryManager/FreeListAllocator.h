#pragma once

class FreeListAllocator
{
public:
	enum class FitPolicy
	{
		BEST_FIT,
		FIRST_FIT
	};
	
	FreeListAllocator(const std::size_t TotalSize, const FitPolicy policy);
	virtual ~FreeListAllocator();

	void* Allocate(const std::size_t AllocationSize, const std::size_t alignment);
	void Deallocate(void* ptr);
	void Reset();

	inline size_t GetTotalAllocatedMemory() const { return m_totalSizeAllocated; }

	FreeListAllocator(const FreeListAllocator&) = delete;
	FreeListAllocator& operator=(const FreeListAllocator&) = delete;
private:
	template <class T>
	class ForwardLinkedList {
	public:
		struct Node {
			T data;
			Node* next;
		};

		Node* head;

	public:
		ForwardLinkedList() = default;

		inline void insert(Node* previousNode, Node* newNode)
		{
			if (previousNode == nullptr) {
				//first node
				if (head != nullptr) {
					newNode->next = head;
				}
				else {
					newNode->next = nullptr;
				}
				head = newNode;
			}
			else {
				if (previousNode->next == nullptr) {
					//last node
					previousNode->next = newNode;
					newNode->next = nullptr;
				}
				else {
					//node in the middle
					newNode->next = previousNode->next;
					previousNode->next = newNode;
				}
			}
		}
		void remove(Node* previousNode, Node* deleteNode)
		{
			if (previousNode == nullptr) {
				//first node
				if (deleteNode->next == nullptr) {
					//list length == 1
					head = nullptr;
				}
				else {
					head = deleteNode->next;
				}
			}
			else {
				previousNode->next = deleteNode->next;
			}
		}
	};
	struct FreeBlockHeader
	{
		std::size_t blockSize;
	};
	struct AllocatedBlockHeader : FreeBlockHeader
	{
		char padding;
	};
	using Node = ForwardLinkedList<FreeBlockHeader>::Node;
	using FreeBlocks = ForwardLinkedList<FreeBlockHeader>;

	const FitPolicy m_policy;
	const std::size_t m_totalSizeAllocated;
	void* mp_start = nullptr;
	FreeBlocks m_freeList;

	FreeListAllocator(FreeListAllocator& freeListAllocator); //disable constructor

	/*Merge up to 3 contiguous free blocks in one*/
	void Coalescence(Node* prevBlock, Node* freeBlock);

	void Find(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
	void FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
	void FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
};

