# ShirosMemoryManager
An implementation of a C++ Memory Manager that is able to respond either to small size allocation request or large ones.

The Memory Manager underlying allocators are:

- For small objects, a SmallObjAllocator based by the solution provided by Andrei Alexandrescu in his book Modern C++ Design: Generic Programming and Design Patterns Applied
- For large objects, a FreeListAllocator with a starting memory pool and implemented using a LinkedList of nodes
