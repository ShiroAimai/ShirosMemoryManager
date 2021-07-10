#pragma once
#include <iostream>
#include "ShirosAllocator.h"

int main()
{
	size_t ObjSize = 32;
	ShirosAllocator Alloc(ObjSize, MAX_SMALL_OBJECT_SIZE);

	return 0;
}