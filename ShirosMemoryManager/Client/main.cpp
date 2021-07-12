#pragma once
#include <iostream>
#include "ShirosSmallObjAllocator.h"
using std::cout;
using std::endl;

struct SmallObjTest {
	long long a;

	SmallObjTest(long long _a) : a(_a) {}
};

int main()
{
	size_t ChunkSize = 64;
	ShirosSmallObjAllocator SmallObjAllocator(ChunkSize, MAX_SMALL_OBJECT_SIZE);
	
	void* ptr = SmallObjAllocator.Allocate(sizeof(SmallObjTest));
	SmallObjTest* p = new (ptr) SmallObjTest(1500);
	cout << p->a << endl;

	return 0;
}