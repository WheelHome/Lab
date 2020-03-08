#ifndef __CELLMSGBUFFER__H__
#define __CELLMSGBUFFER__H__

#include "Cell.hpp"

class CellMsgBuffer
{
private:
    //Buf
    char* _pBuf = nullptr;
    //The current msgBuf end
    long unsigned int _nLast = 0;
    //Buffer length
    long unsigned int _nSize = 0;
    //Buffer is full.
    int _buffFullCount = 0;
public:
    CellMsgBuffer(int nSize = 8192)
    {
        _nSize = nSize;
        _pBuf = new char[81920];
    }

    ~CellMsgBuffer()
    {
        if(_pBuf)
        {
            delete[] _pBuf;
            _pBuf = nullptr;
        }
    }

    bool push(const char* pData,int nLen)
    {
        if(_nLast + nLen >= _nSize)
        {
            int n = _nLast + nLen - _nSize;
            if(n < 8192)
            {
                n = 8192;
            }
            char* buff = new char[_nSize + n];
            memcpy(buff,_pBuf,_nLast);
            delete[] _pBuf;
            _pBuf = buff;
            _nSize += n;
        }
        //Copy sending data to sendBuf's tail
        memcpy(_pBuf + _nLast,pData,nLen);
        //Move tail of sendBuf to remaining array place
        _nLast += nLen;
        if(_nLast == SEND_BUFF_SIZE)
        {
            _buffFullCount++;
           return false;
        }
        return true;
    }

    int writeToSocket(SOCKET sockfd)
    {
        int ret = 0;
        if(_nLast > 0 && INVALID_SOCKET != sockfd)
        {
            ret = send(sockfd,_pBuf,_nLast,0);
            if(ret > 0 )
            {
                _nLast = 0;
                _buffFullCount = 0;
            }
        }
        return ret;
    }

    int readFromSocket(SOCKET sockfd)
    {
        if(_nSize - _nLast > 0)
        {
            char* szRecv = _pBuf + _nLast;
            int nLen = recv(sockfd,szRecv,_nSize - _nLast,0);
            if(nLen < 0)
            {
                //std::cout << "Client socket = " << pClient->getSockfd() << " quited." << std::endl;
                return nLen;
            }
            _nLast += nLen;
            return nLen;
        }
        return 0;
    }

    bool hasMsg()
    {
        //Received a integrated msgHeader
        if(_nLast  >= sizeof(netmsg_DataHeader))
        {
            //There can be know the all msgData
            netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuf;
            //Received a integrated msgData
            return _nLast >= header->dataLength;
        }
        return false;
    }

    bool needWrite()
    {
        return _nLast > 0;
    }

    char* data()
    {
        return _pBuf;
    }

    void pop(int nLen)
    {
        int n = _nLast - nLen;
        if(n > 0)
        {
            memcpy(_pBuf,_pBuf + nLen,n);
        }
        _nLast = n;
        if(_buffFullCount > 0)
        {
            --_buffFullCount;
        }
    }
};


#endif  //!__CELLMSGBUFFER__H__