#ifndef _CLIENT_SOCKET_H_
#define _CLIENT_SOCKET_H_


#include "Cell.hpp"
#include "CELLObjPoll.hpp"

#define CLIENT_HEART_DEAD_TIME 5000
//Client data type
class CellClient : public ObjectPollBase<CellClient,1000>
{
private:
    SOCKET _sockfd;//socket fd_set desc set
    //msgBuf
    char _szMsgBuf[RECV_BUFF_SIZE] = {};
    //The msgBuf end
    long unsigned int _lastPos = 0;

    //sendBuf
    char _szSendBuf[SEND_BUFF_SIZE] = {};
    //The msgBuf end
    long unsigned int _lastSendPos = 0;

    //Heart beat death time
    time_t _dtHeart;

public:
    bool checkHeart(time_t dt)
    {
        _dtHeart += dt;
        if(_dtHeart >= CLIENT_HEART_DEAD_TIME)
        {
            std::cout << "CheckHeart dead:" << _sockfd << " time:" << _dtHeart << std::endl;
            return true;
        }
        return false;
    }

    void resetDTHeart()
    {
        _dtHeart = 0;
    }

    CellClient(SOCKET _sockfd = INVALID_SOCKET)
    {
        this->_sockfd = _sockfd;
        bzero(_szMsgBuf,sizeof(_szMsgBuf));
        this->_lastPos = 0;

        bzero(_szSendBuf,sizeof(_szSendBuf));
        this->_lastSendPos = 0;

        resetDTHeart();
    }

    SOCKET getSockfd()
    {
        if(_sockfd!=SOCKET_ERROR)
            return _sockfd;
        return SOCKET_ERROR;
    }

    char* getMsgBuf()
    {
        return _szMsgBuf;
    }

    long unsigned int getLastPos()
    {
        return _lastPos;
    }
    
    void setLastPos(int newPos)
    {
        this->_lastPos = newPos;
    }

    int sendData(DataHeader* header)
    {
        int ret = SOCKET_ERROR;
        int nSendLen = header->dataLength;
        const char* pSendData = (const char*)header;
        while(true)
        {
            if(_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
            {
                //calculate remain place
                int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
                memcpy(_szSendBuf + _lastSendPos,pSendData,nCopyLen);
                //Remain data
                pSendData += nCopyLen;
                //Remain data length
                nSendLen -= nCopyLen;
                if( header && _sockfd)
                {
                    sigset_t set;
                    sigemptyset(&set);
                    sigaddset(&set, SIGPIPE);
                    sigprocmask(SIG_BLOCK, &set, NULL); 
                    ret = send(_sockfd,_szSendBuf,SEND_BUFF_SIZE,0);
                    //set tail of data to zero
                    _lastSendPos = 0;
                    if(ret == SOCKET_ERROR)
                    {
                        return ret;
                    }
                }
            }else
            {
                //Copy sending data to sendBuf's tail
                memcpy(_szSendBuf + _lastSendPos,pSendData,nSendLen);
                //Move tail of sendBuf to remaining array place
                _lastSendPos += nSendLen;
                break;
            }
        }
        return ret;
    }
};

typedef  std::shared_ptr<CellClient> ClientSocketPtr;

#endif