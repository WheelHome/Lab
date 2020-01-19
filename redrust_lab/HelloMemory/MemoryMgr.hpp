#ifndef __MEMORYMGR__H__
#define __MEMORYMGR__H__

#include <stdlib.h>
//Memory minium block 
class MemoryBlock
{
private:
    int nID;
    int nRef;
    MemoryAlloc* pAlloc;
    MemoryBlock* pNext;
    bool bPool;
    char cNull[3];
public:
    MemoryBlock(/* args */)
    {

    }

    ~MemoryBlock()
    {

    }
};

//MemoryPoll
class MemoryAlloc
{
private:
    //MemoryPollAddress
    char* _pBuf;
    //MemoryPollHeadBlock
    MemoryBlock* pHeader;
    //MemoryBlockSize
    size_t _nSize;
    //MemoryBlockNum
    size_t _nBlockNum;
public:
    MemoryAlloc(/* args */)
    {

    }

    ~MemoryAlloc()
    {

    }

    //getMemory
    void* alloc(size_t nSize)
    {
        return malloc(nSize);
    }

    //delMemory
    void freeMem(void* p)
    {
        free(p);
    }

    //Init Memory poll
    void initMemory()
    {
        //get memory from system
        //init memory poll
    }
};

//MemoryManageToll
class MemoryMgr
{
private:
    /* data */

    MemoryMgr(/* args */)
    {
        
    }

    ~MemoryMgr()
    {

    }

public:
    //Single instance
    static  MemoryMgr& Instance()
    {
        static MemoryMgr mgr;
        return mgr;
    }

    //getMemory
    void* alloc(size_t nSize)
    {
        return malloc(nSize);
    }

    //delMemory
    void freeMem(void* p)
    {
        free(p);
    }
};

#endif  //!__MEMORYMGR__H__