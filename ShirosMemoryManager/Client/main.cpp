#pragma once

//TEST MODES
//#define GLOBAL_OP_OVERLOAD
//#define MM_PERFORMANCE
#define STL_ALLOCATOR
//#define BOTH_ALLOC_USED
//#define ARRAY_TEST

#include <iostream>
#include "ShirosMemoryManager.h"
#include "ShirosSTLAllocator.h"
#include <ctime>
#include <chrono>
#include <cassert>

#ifdef GLOBAL_OP_OVERLOAD
#define GLOBAL_SHIRO_MM
#include "GlobalShirosMemoryManager.h"
#endif

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

using std::cout;
using std::endl;

//24 bytes
struct SmallObjTest {
	long long a;
	float b;
	float d;
	short c;
};

//2052 bytes
struct LargeObjTest {
	long a;
	char b[2048];
};

void MMPerformanceTest()
{
	cout << "====== MM PERFORMANCE TEST ======" << endl;

	std::vector<void*> PointersToSmallObjTest;
	auto start_millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < 1000000; ++i)
	{
		void* ptr = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
		SmallObjTest* p = new (ptr) SmallObjTest();
		PointersToSmallObjTest.push_back(ptr);
	}
	for (int i = 0; i < 1000000; ++i)
	{
		MM_DELETE(PointersToSmallObjTest[i], sizeof(SmallObjTest));
	}
	auto end_millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	long long delta = end_millisec - start_millisec;
	cout << "SmallObjAllocator takes :" << std::to_string((float)delta / 1000) << " to complete" << endl; //convert to seconds

	std::vector<SmallObjTest*> PointersToSmallObjTest2;

	start_millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < 1000000; ++i)
	{
		SmallObjTest* ptr = new SmallObjTest();
		PointersToSmallObjTest2.push_back(ptr);
	}
	for (int i = 0; i < 1000000; ++i)
	{
		delete PointersToSmallObjTest2[i];
	}
	end_millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	delta = end_millisec - start_millisec;
	cout << "Default Allocator takes :" << std::to_string((float)delta / 1000) << " to complete" << endl; //convert to seconds

	cout << "====== END OF MM PERFORMANCE TEST ======" << endl;
}

void CheckMemoryLeak() {
	cout << "====== MEMORY LEAK TEST ======" << endl;
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();

	Instance.PrintMemoryState();
	
	{
		MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	}

	Instance.PrintMemoryState();
	
	assert(Instance.GetCurrentlyUsedMemory() && Instance.GetMemoryRequested() > Instance.GetMemoryFreed() && "Memory Leak identified");

	Instance.Reset();

	Instance.PrintMemoryState();

	cout << "====== END OF MEMORY LEAK TEST ======" << endl;
}

void CheckStandardBehavior()
{
	cout << "====== STANDARD BEHAVIOR TEST ======" << endl;
	ShirosMemoryManager& Instance = ShirosMemoryManager::Get();

	Instance.PrintMemoryState();
	void* ptr = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	Instance.PrintMemoryState();
	MM_DELETE(ptr, sizeof(SmallObjTest));
	
	assert(Instance.GetCurrentlyUsedMemory() == 0 && Instance.GetMemoryRequested() == Instance.GetMemoryFreed() && "Memory Allocated and deallocated correctly");
	Instance.PrintMemoryState();

	cout << "====== END OF STANDARD BEHAVIOR TEST ======" << endl;
}

int main()
{

	CheckMemoryLeak();
	CheckStandardBehavior();

#ifdef MM_PERFORMANCE
	MMPerformanceTest();
#endif
#ifdef GLOBAL_OP_OVERLOAD
	ShirosMemoryManager::Get().PrintMemoryState();
	void* ptr = new SmallObjTest();
	void* ptr2 = new SmallObjTest();
	void* ptr3 = new SmallObjTest();
	void* ptr4 = new SmallObjTest();
	void* ptr5 = new SmallObjTest();
	void* ptr6 = new SmallObjTest();
	void* ptr7 = new SmallObjTest();
	void* ptr8 = new SmallObjTest();
	void* ptr9 = new SmallObjTest();
	void* ptr10 = new SmallObjTest();
	ShirosMemoryManager::Get().PrintMemoryState();
	::operator delete(ptr, sizeof(SmallObjTest));
	::operator delete(ptr2, sizeof(SmallObjTest));
	::operator delete(ptr3, sizeof(SmallObjTest));
	::operator delete(ptr4, sizeof(SmallObjTest));
	::operator delete(ptr5, sizeof(SmallObjTest));
	::operator delete(ptr6, sizeof(SmallObjTest));
	::operator delete(ptr7, sizeof(SmallObjTest));
	::operator delete(ptr8, sizeof(SmallObjTest));
	::operator delete(ptr9, sizeof(SmallObjTest));
	::operator delete(ptr10, sizeof(SmallObjTest));
	ShirosMemoryManager::Get().PrintMemoryState();
#endif
#ifdef BOTH_ALLOC_USED
	ShirosMemoryManager::Get().PrintMemoryState();
	void* ptr = MM_NEW(sizeof(LargeObjTest), alignof(LargeObjTest));
	void* ptr1 = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	void* ptr2 = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	void* ptr3 = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	void* ptr4 = MM_NEW(sizeof(LargeObjTest), alignof(LargeObjTest));
	void* ptr5 = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	void* ptr6 = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	void* ptr7 = MM_NEW(sizeof(SmallObjTest), alignof(SmallObjTest));
	void* ptr8 = MM_NEW(sizeof(LargeObjTest), alignof(LargeObjTest));
	ShirosMemoryManager::Get().PrintMemoryState();
	MM_DELETE(ptr, sizeof(LargeObjTest));
	MM_DELETE(ptr1, sizeof(SmallObjTest));
	MM_DELETE(ptr2, sizeof(SmallObjTest));
	MM_DELETE(ptr3, sizeof(SmallObjTest));
	MM_DELETE(ptr4, sizeof(LargeObjTest));
	MM_DELETE(ptr5, sizeof(SmallObjTest));
	MM_DELETE(ptr6, sizeof(SmallObjTest));
	MM_DELETE(ptr7, sizeof(SmallObjTest));
	MM_DELETE(ptr8, sizeof(LargeObjTest));
	ShirosMemoryManager::Get().PrintMemoryState();
#endif
#ifdef ARRAY_TEST
	ShirosMemoryManager::Get().PrintMemoryState();
	void* ptr = MM_NEW_A(20, sizeof(LargeObjTest), alignof(LargeObjTest));
	void* ptr1 = MM_NEW_A(5, sizeof(SmallObjTest), alignof(SmallObjTest));
	ShirosMemoryManager::Get().PrintMemoryState();
	MM_DELETE_A(ptr1, 5, sizeof(SmallObjTest));
	MM_DELETE_A(ptr, 20, sizeof(LargeObjTest));
	ShirosMemoryManager::Get().PrintMemoryState();
#endif
#ifdef STL_ALLOCATOR
	std::vector<SmallObjTest, ShirosSTLAllocator<SmallObjTest>> a;
	ShirosMemoryManager::Get().PrintMemoryState();
	a.push_back(SmallObjTest());
	ShirosMemoryManager::Get().PrintMemoryState();
	a.push_back(SmallObjTest());
	ShirosMemoryManager::Get().PrintMemoryState();
	a.push_back(SmallObjTest());
	ShirosMemoryManager::Get().PrintMemoryState();
	a.clear();
#endif
	return 0;
}