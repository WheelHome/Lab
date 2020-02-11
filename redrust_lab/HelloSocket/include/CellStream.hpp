#ifndef __CELLSTREAM__H__
#define __CELLSTREAM__H__

#include <cstdint>

#include "Cell.hpp"
class CellStream
{
private:
    //Buf
    char* _pBuf = nullptr;
    //The current msgBuf end
    long unsigned int _nWritePos = 0;
    //Buffer length
    long unsigned int _nSize = 0;
    long unsigned int _nReadPos = 0;
    bool _bDelete  = true;
public:
    CellStream(char* pData,int nSize,bool bDelete = false)
    {
        _nSize = nSize;
        _pBuf = pData;
        _bDelete = bDelete;
    }

    CellStream(int nSize = 1024)
    {
        _nSize = nSize;
        _pBuf = new char[_nSize];
    }

    virtual ~CellStream()
    {
        if(_bDelete&&_pBuf)
        {
            delete[] _pBuf;
            _pBuf = nullptr;
        }
    }

    inline void push(int n)
    {
        _nReadPos += n;
    }

    inline void pop(int n)
    {
        _nWritePos += n;
    }

    inline bool canRead(int n)
    {
        return _nSize - _nReadPos >= n;
    }

    inline bool canWrite(int n)
    {
        return _nSize - _nWritePos >= n;
    }

    inline void setWritePos(int n)
    {
        _nWritePos = n;
    }

    inline void setReadPos(int n)
    {
        _nReadPos = n;
    }

    bool readInt8(int8_t& def)  //char
    {
        return read(def);
    }

    bool readInt16(int16_t& def) //short
    {
        return read(def);
    }

    bool readInt32(int32_t& def) //int
    {
        return read(def);
    }

    bool readFloat(float& def)
    {
        return read(def);
    }

    bool readDouble(double& def)
    {

        return read(def);
    }

    bool writeInt8(int8_t n)    //char
    {

        return write<int8_t>(n);
    }

     bool writeInt16(int16_t n)   //short
    {

        return write<int16_t>(n);
    }

    bool writeInt32(int32_t n)   //int
    {
        return write<int32_t>(n);
    }

    bool writeFloat(float n)
    {
        return write<float>(n);
    }

    bool writeDouble(double n)
    {
        return write<double>(n);
    }

    template<typename T>
    bool writeArray(T* pData,uint32_t len)
    {
        size_t nLen = sizeof(T) * len;
        if(_nWritePos + nLen + sizeof(uint32_t) <= _nSize)
        {
            writeInt32(len);
            //Copy sending data to sendBuf's tail
            memcpy(_pBuf + _nWritePos,pData,nLen);
            //Move tail of sendBuf to remaining array place
            _nWritePos += nLen;
            return true;
        }
        return false;
    }

    template<typename T>
    bool justRead(T& n)
    {
        return Read(n,false);
    }

    template<typename T>
    uint32_t readArray(T* pArr,uint32_t& len)
    {
        uint32_t len1 = 0;
        read(len1,false);
        if(len1 < len)
        {
            auto nLen = len1 * sizeof(T);
            if(_nReadPos + nLen + sizeof(uint32_t) <= _nSize)
            {
                _nReadPos += sizeof(uint32_t);
                memcpy(pArr,_pBuf + _nReadPos,nLen);
                _nReadPos += nLen;
                return len1;
            }
        }
        _nReadPos -= sizeof(uint32_t);
        return 0;
    }
    
    template<typename T>
    bool write(T n)   //int
    {
        size_t nLen = sizeof(T);
        if(_nWritePos + nLen <= _nSize)
        {
            memcpy(_pBuf + _nWritePos,&n,nLen);
            _nWritePos += nLen;
            return true;
        }
        return false;
    }

    template<typename T>
    bool read(T& n,bool bOffset = true)
    {
        auto nLen = sizeof(T);
        if(_nReadPos + nLen <= _nSize)
        {
            memcpy(&n,_pBuf + _nReadPos,nLen);
            if(bOffset)
            {
               _nReadPos += nLen;
            }
            return true;
        }
        return false;
    }
    
    char* data()
    {
        return _pBuf;
    }

    int length()
    {
        return _nWritePos;
    }
    

};

#endif  //!__CELLSTREAM__H__