#ifndef __MEMORYMGR__H__
#define __MEMORYMGR__H__

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mutex>

#ifndef _DEBUG
#include <stdio.h>
    #define xPrintf(...) printf(__VA_ARGS__)
#else
    #define xPrintf(...)
#endif  //!_DEBUG

#define MEMORY_SIZE_64 64
#define MEMORY_SIZE_128 128
#define  MEMORY_SIZE_256 256
#define  MEMORY_SIZE_512 512  
#define  MEMORY_SIZE_1024 1024  
#define  MEMORY_BLOCK_NUM 10000  

#define  MAX_MEMORY_SIZE MEMORY_SIZE_128  

class MemoryAlloc;
//Memory minium block 
class MemoryBlock
{
public:
    int nID;
    int nRef;
    MemoryAlloc* pAlloc;
    MemoryBlock* pNext;
    bool bPool;
    char cNull[3];
public:
    MemoryBlock(/* args */){   }

    ~MemoryBlock(){   }
};

//MemoryPoll
class MemoryAlloc
{
protected:
    //MemoryPollAddress
    char* _pBuf;
    //MemoryPollHeadBlock
    MemoryBlock* _pHeader;
    //MemoryBlockSize
    size_t _nSize;
    //MemoryBlockNum
    size_t _nBlockNum;
    std::mutex _mutex;
public:
    MemoryAlloc(/* args */)
    {
        _pBuf = nullptr;
        _pHeader = nullptr;
        _nSize = 0;
        _nBlockNum = 0;
    }

    ~MemoryAlloc()
    {
        if(_pBuf)
        {
            free(_pBuf);
        }
    }

    //getMemory
    void* allocMemory(size_t nSize)
    {
		std::lock_guard<std::mutex> lg(_mutex);
        if(_pBuf == nullptr)
        {
            initMemory();
        }
        MemoryBlock* pReturn = nullptr;
        if(!_pHeader)
        {
            pReturn = (MemoryBlock*)malloc(sizeof(MemoryBlock) + nSize);
            pReturn->bPool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pAlloc = nullptr;
            pReturn->pNext = nullptr;
        }else
        {
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        xPrintf("allocMemory: %llx,id=%d,size=%d\n",(long long unsigned int)pReturn,pReturn->nID,(int)nSize);
        return ((char*)pReturn + sizeof(MemoryBlock));
    }

    //delMemory
    void freeMemory(void* pMem)
    {
        MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
        xPrintf("freeMemory: %llx,id=%d\n",(long long unsigned int)pBlock,pBlock->nID);
        if(pBlock->bPool)
        {
            std::lock_guard<std::mutex> lg(_mutex);
            if(--pBlock->nRef != 0)
            {
                return ;
            }
            pBlock->pNext = _pHeader;
            _pHeader = pBlock;
            }else
            {
                if(--pBlock->nRef != 0)
                {
                    return ;
                }
                free(pBlock);    
            }
       }

    //Init Memory poll
    void initMemory()
    {
        assert(_pBuf == nullptr);
        if(_pBuf)
        {
            return ;
        }

        //get memory from system
        size_t realSize = _nSize + sizeof(MemoryBlock);
        size_t bufSize = realSize * _nBlockNum;
        _pBuf = (char*)malloc(bufSize);
        bzero(_pBuf,bufSize);

        //init memory poll
        _pHeader = (MemoryBlock*)_pBuf;
        _pHeader->bPool = true;
        _pHeader->nID = 0;
        _pHeader->nRef = 0;
        _pHeader->pAlloc = this;
        _pHeader->pNext = nullptr;

        MemoryBlock* Temp = _pHeader;
        for(size_t n = 1; n < _nBlockNum ; n++)
        {
            MemoryBlock* pTemp = (MemoryBlock*)((char*)_pBuf + n * realSize);
            pTemp->bPool = true;
            pTemp->nID = n;
            pTemp->nRef = 0;
            pTemp->pAlloc = this;
            pTemp->pNext = nullptr;

            Temp->pNext = pTemp;
            Temp=pTemp;
        }
    }


};

template<size_t nSize,size_t nBlockNum>
class MemoryAllocator : public MemoryAlloc
{
private:
    /* data */
public:
    MemoryAllocator (/* args */)
    {
        const size_t n = sizeof(void*);
        _nSize = (nSize / n) * n + (nSize % n ? n : 0);
        _nBlockNum = nBlockNum;
    }

    ~MemoryAllocator ()
    {

    }
};


//MemoryManageToll
class MemoryMgr
{
private:
    MemoryAllocator<MEMORY_SIZE_64,MEMORY_BLOCK_NUM> _mem64;
    MemoryAllocator<MEMORY_SIZE_128,MEMORY_BLOCK_NUM> _mem128;
    //MemoryAllocator<MEMORY_SIZE_256,MEMORY_BLOCK_NUM> _mem256;
    //MemoryAllocator<MEMORY_SIZE_512,MEMORY_BLOCK_NUM> _mem512;
    //MemoryAllocator<MEMORY_SIZE_1024,MEMORY_BLOCK_NUM> _mem1024;
    MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1] ;

    MemoryMgr(/* args */)
    {
        init_szAlloc(0,64,&_mem64);
        init_szAlloc(65,128,&_mem128);
        //init_szAlloc(129,256,&_mem256);
        //init_szAlloc(257,512,&_mem512);
        //init_szAlloc(513,1024,&_mem1024);
        std::cout << "MemoryMgr" << std::endl;
    }

    ~MemoryMgr()
    {

    }

    //Init memory poll mapping array
    void init_szAlloc(int nBegin,int nEnd,MemoryAlloc* pMemA)
    {
        for(int n = nBegin; n <= nEnd; n++)
        {
            _szAlloc[n] = pMemA;
        }
    }

public:
    //Single instance
    static  MemoryMgr& Instance()
    {
        static MemoryMgr mgr;
        return mgr;
    }

    //getMemory
    void* allocMem(size_t nSize)
    {
        if(nSize <= MAX_MEMORY_SIZE)
        {
            return _szAlloc[nSize]->allocMemory(nSize);
        }else
        {
            MemoryBlock* pReturn = (MemoryBlock*)malloc( sizeof(MemoryBlock) + nSize);
            pReturn->bPool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pAlloc = nullptr;
            pReturn->pNext = nullptr;
            xPrintf("allocMem: %llx,id=%d,size=%d\n",(long long unsigned int)pReturn,pReturn->nID,(int)nSize);
            return ((char*)pReturn + sizeof(MemoryBlock));
        }
    }

    //delMemory
    void freeMem(void* pMem)
    {
        MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
        xPrintf("freeMem: %llx,id=%d,bPool=%d\n",(long long unsigned int)pBlock,pBlock->nID,pBlock->bPool);
        if(pBlock->bPool)
        {
            pBlock->pAlloc->freeMemory(pMem);
        }
        else
        {
            if(--pBlock->nRef == 0)
            {
                free(pBlock);
            }
        }
    }

    //Add memory block reference count
    void addRef(void* pMem)
    {
        MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
        ++pBlock->nRef;
    }
};

#endif  //!__MEMORYMGR__H__