
#include "Allocator.h"
#include "MemoryMgr.hpp"
#include <iostream>
void* operator new(size_t size)
{
    return MemoryMgr::Instance().allocMem(size);
}


void* operator new[](size_t size)
{
    return MemoryMgr::Instance().allocMem(size);
}

void operator delete(void* p) noexcept
{
    return MemoryMgr::Instance().freeMem(p);
}

void operator delete[](void* p) noexcept
{
    return MemoryMgr::Instance().freeMem(p);
}


void* mem_alloc(size_t size)
{
    return malloc(size);
}

void mem_free(void* p)
{
    free(p);
}