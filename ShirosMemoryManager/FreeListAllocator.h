#pragma once

using std::size_t;

class FreeListAllocator
{
public:
	enum class FitPolicy
	{
		BEST_FIT,
		FIRST_FIT
	};
	
	FreeListAllocator(size_t TotalSize, FitPolicy policy);
	~FreeListAllocator();

	void* Allocate(size_t AllocationSize, size_t alignment, size_t& OutAllocationSize);
	size_t Deallocate(void* ptr);
	void Reset();

	inline size_t GetTotalAllocatedMemory() const { return m_totalSizeAllocated; }

	/** Prevent copy for this class */
	FreeListAllocator(const FreeListAllocator&) = delete;
	FreeListAllocator& operator=(const FreeListAllocator&) = delete;
private:
	/** Internal forward linked list */
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
		inline void remove(Node* previousNode, Node* deleteNode)
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
	/** Internal struct identifying a free block */
	struct FreeBlockHeader
	{
		size_t blockSize;
	};
	/** Internal struct identifying an allocated block */
	struct AllocatedBlockHeader : FreeBlockHeader
	{
		char padding;
	};
	using Node = ForwardLinkedList<FreeBlockHeader>::Node;
	using FreeBlocks = ForwardLinkedList<FreeBlockHeader>;

	/** Selected FitPolicy*/
	const FitPolicy m_policy;
	/** Tracked memory allocated by this allocator*/
	const size_t m_totalSizeAllocated;
	/** Internal pointer pointing to the first address of the memory pool*/
	void* mp_start = nullptr;
	/** ForwardLinkedList tracking FreeBlock in list*/
	FreeBlocks m_freeList;

	FreeListAllocator(FreeListAllocator& freeListAllocator); //disable constructor

	void Release();
	/*Merge up to 3 contiguous free blocks in one*/
	void Coalescence(Node* prevBlock, Node* freeBlock);
	/** Find method that will apply the alghoritm matching the desired FitPolicy */
	//TODO : Modify FitPolicy selection algorithm maybe using a factory method
	void Find(size_t size, size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
	/** Best fit policy. Find the best freeblock to use among all the blocks. Time complexity is O(N), where N is the number of free blocks */
	void FindBest(size_t size, size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
	/** First fit policy. Find the first freeblock able to handle the requested size. Time complexity is O(N), where N is the number of free blocks */
	void FindFirst(size_t size, size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
};

