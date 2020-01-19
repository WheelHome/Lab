#ifndef __ALLOCATOR__H__
#define __ALLOCATOR__H__

void* operator new(size_t size);

void* operator new[](size_t size);

void operator delete(void* p);

void operator delete(void* p);

void* mem_alloc(size_t size);

void mem_free(void* p);
#endif  //!__ALLOCATOR__H__