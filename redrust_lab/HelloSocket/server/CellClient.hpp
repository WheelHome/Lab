#ifndef _CLIENT_SOCKET_H_
#define _CLIENT_SOCKET_H_


#include "Cell.hpp"
#include "CellObjPoll.hpp"

#define CLIENT_HEART_DEAD_TIME 60000
#define CLIENT_SEND_BUFF_TIME 5000

//Client data type
class CellClient : public ObjectPollBase<CellClient,1000>
{
public:
    int id = -1;
    int _serverId = -1;
private:
    SOCKET _sockfd;//socket fd_set desc set
    //msgBuf
    char _szMsgBuf[RECV_BUFF_SIZE] = {};

    //sendBuf
    char _szSendBuf[SEND_BUFF_SIZE] = {};
    //The msgBuf end
    long unsigned int _lastSendPos = 0;
    //The msgBuf end
    long unsigned int _lastPos = 0;
    int _sendBuffFullCount = 0;

    //Heart beat death time
    time_t _dtHeart;
    //last send msg data time
    time_t _dtSend;

    std::mutex _mutex;

public:

    int sendDataImme(netmsg_DataHeader* header)
    {
        sendData(header);
        return sendDataImme();
    }

    int sendDataImme()
    {
        int ret = 0;
        if(_lastPos > 0 && INVALID_SOCKET != _sockfd)
        {
            ret = send(_sockfd,_szSendBuf,_lastPos,0);
            _lastPos = 0;
            _sendBuffFullCount = 0;
            resetDTSend();
        }
        return ret;
    }

    bool checkSend(time_t dt)
    {
        _dtSend += dt;
        if(_dtSend >= CLIENT_SEND_BUFF_TIME)
        {
            //std::cout << "CheckSend:" << _sockfd << " time:" << _dtSend << std::endl;
            //immediately send  sendBuf data 
            sendDataImme();
            //reset send time
            return true;
        }
        return false;
    }

    void resetDTSend()
    {
        _dtSend = 0;
    }

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
        static int n = 1;
        id = n++;
        this->_sockfd = _sockfd;
        bzero(_szMsgBuf,sizeof(_szMsgBuf));
        this->_lastPos = 0;

        bzero(_szSendBuf,sizeof(_szSendBuf));
        this->_lastSendPos = 0;

        resetDTHeart();
        resetDTSend();
    }

    ~CellClient()
    {
        std::cout << "s: " << _serverId << " CellClient: "<< id <<"~CellClient() 1" << std::endl;
        if(_sockfd != INVALID_SOCKET)
        {
           closesocket(_sockfd);
        }
        _sockfd = INVALID_SOCKET;
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

    int sendData(netmsg_DataHeader* header)
    {
        int ret = 0;
        int nSendLen = header->dataLength;
        const char* pSendData = (const char*)header;
        if(_lastSendPos + nSendLen <= SEND_BUFF_SIZE)
        {
            //Copy sending data to sendBuf's tail
            memcpy(_szSendBuf + _lastSendPos,pSendData,nSendLen);
            //Move tail of sendBuf to remaining array place
            _lastSendPos += nSendLen;
            if(_lastSendPos == SEND_BUFF_SIZE)
            {
                _sendBuffFullCount++;
            }
            return nSendLen;
        }else
        {
            _sendBuffFullCount++;
        }
        return ret;
    }
};

typedef  std::shared_ptr<CellClient> ClientSocketPtr;

#endif