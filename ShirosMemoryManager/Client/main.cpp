#pragma once

//TEST MODES
#define GLOBAL_OP_OVERLOAD
//#define MM_PERFORMANCE
//#define STL_ALLOCATOR

#include <iostream>
#include "ShirosMemoryManager.h"
#include "ShirosSTLAllocator.h"
#include <ctime>
#include <chrono>

#ifdef GLOBAL_OP_OVERLOAD
#define GLOBAL_SHIRO_MM
#endif

#include "GlobalShirosMemoryManager.h"
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

using std::cout;
using std::endl;

struct SmallObjTest {
	long long a;
	float b;
	float d;
	short c;
};

void MMPerformanceTest()
{
	std::vector<void*> PointersToSmallObjTest;
	auto start_millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < 1000000; ++i)
	{
		void* ptr = MM_NEW(sizeof(SmallObjTest));
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
}

int main()
{
#ifdef MM_PERFORMANCE
	MMPerformanceTest();
#endif
#ifdef GLOBAL_OP_OVERLOAD

	ShirosMemoryManager::PrintMemoryState();
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
	ShirosMemoryManager::PrintMemoryState();
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
	ShirosMemoryManager::PrintMemoryState();
#endif
#ifdef STL_ALLOCATOR
	std::vector<SmallObjTest, ShirosSTLAllocator<SmallObjTest>> a;
	a.push_back(SmallObjTest());
#endif

	return 0;
}