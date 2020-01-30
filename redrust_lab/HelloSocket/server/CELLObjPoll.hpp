#ifndef __CELLOBJPOLL__H__
#define __CELLOBJPOLL__H__
#include <cstdlib>
#include <mutex>
#include <assert.h>


struct NodeHeader
{
    NodeHeader* pNext;
    int nID;
    int nRef;
    bool bPool;
    char cNull[2];
};

template<class Type,size_t nPollNum>
class CELLObjectPoll
{
private:
    NodeHeader* _pHeader;
    char* _pBuf;
    std::mutex _mutex;  
    //Init ObjectPool
    void InitPoll()
    {
        assert(_pBuf == nullptr);
        if(_pBuf)
        {
            return ;
        }
        //Calculate the Object Poll size
        size_t realSize = sizeof(Type) + sizeof(NodeHeader);
        size_t n = nPollNum * realSize;
        _pBuf = new char[n];

        //Init Object Poll
        _pHeader = (NodeHeader*)_pBuf;
        _pHeader->bPool = true;
        _pHeader->nID = 0;
        _pHeader->nRef = 0;
        _pHeader->pNext = nullptr;

        NodeHeader* Temp = _pHeader;
        for(size_t n = 1; n < nPollNum ; n++)
        {
            NodeHeader* pTemp = (NodeHeader*)((char*)_pBuf + n * realSize);
            pTemp->bPool = true;
            pTemp->nID = n;
            pTemp->nRef = 0;
            pTemp->pNext = nullptr;

            Temp->pNext = pTemp;
            Temp=pTemp;
        }
    }

public:
    //Get Object
    void* allocObjMemory(size_t size)
    {
		std::lock_guard<std::mutex> lg(_mutex);
        NodeHeader* pReturn = nullptr;
        if(!_pHeader)
        {
            pReturn =(NodeHeader*) new char[sizeof(Type) + sizeof(NodeHeader)];
            pReturn->bPool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pNext = nullptr;
        }else
        {
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        printf("allocObjMemory: %llx,id=%d,size=%d\n",(long long unsigned int)pReturn,pReturn->nID,(int)size);
        return ((char*)pReturn + sizeof(NodeHeader));
    }

    //Delete Object
    void freeObjMemory(void* pMem)
    {
        NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
        printf("freeObjMemory: %llx,id=%d\n",(long long unsigned int)pBlock,pBlock->nID);
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
            delete[] pBlock; 
        }
    }

    CELLObjectPoll(/* args */)
    {
        _pBuf = nullptr;
        InitPoll();
    }

    ~CELLObjectPoll()
    {
        if(_pBuf)
        {
            delete[] _pBuf;
        }
    }
};

template<class Type,size_t nPollNum>
class ObjectPollBase
{
private:
    typedef CELLObjectPoll<Type,nPollNum> ClassTypePoll;
    static ClassTypePoll&  objectPoll()
    {
        static ClassTypePoll sPoll;
        return sPoll;
    }

public:
    ObjectPollBase(/* args */)
    {

    }

    ~ObjectPollBase()
    {

    }

    void* operator new(size_t size)
    {
        return objectPoll().allocObjMemory(size);
    }

    void operator delete(void* p) noexcept
    {
        objectPoll().freeObjMemory(p);
    }

    template<typename ...Args>
    static Type* createObj(Args ... args)
    {
        Type* obj = new Type(args...);
        return obj;
    }

    static void destoryObject(Type* obj)
    {
        delete obj;
    }
};

#endif  //!__CELLOBJPOLL__H__