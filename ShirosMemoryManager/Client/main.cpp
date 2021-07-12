#pragma once
#include <iostream>
#include "ShirosSmallObjAllocator.h"
#include <ctime>
#include <chrono>

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

void PerformanceTest(ShirosSmallObjAllocator& SmallObjAllocator)
{
	std::vector<void*> PointersToSmallObjTest;
	auto start_millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < 1000000; ++i)
	{
		void* ptr = SmallObjAllocator.Allocate(sizeof(SmallObjTest));
		SmallObjTest* p = new (ptr) SmallObjTest();
		PointersToSmallObjTest.push_back(ptr);
	}
	for (int i = 0; i < 1000000; ++i)
	{
		SmallObjAllocator.Deallocate(PointersToSmallObjTest[i], sizeof(SmallObjTest));
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
	ShirosSmallObjAllocator SmallObjAllocator(DEFAULT_CHUNK_SIZE);
	PerformanceTest(SmallObjAllocator);

	return 0;
}