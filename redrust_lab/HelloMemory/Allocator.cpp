
#include "Allocator.h"
#include "MemoryMgr.hpp"

void* operator new(size_t size)
{
    return MemoryMgr::Instance().alloc(size);
}


void* operator new[](size_t size)
{
    return MemoryMgr::Instance().alloc(size);
}

void operator delete(void* p)
{
    return MemoryMgr::Instance().freeMem(p);
}

void operator delete(void* p)
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