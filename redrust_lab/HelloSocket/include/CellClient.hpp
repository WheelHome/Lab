#ifndef _CLIENT_SOCKET_H_
#define _CLIENT_SOCKET_H_


#include "Cell.hpp"
#include "CellObjPoll.hpp"
#include "CellMsgBuffer.hpp"

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
    CellMsgBuffer _sendBuffer;
    CellMsgBuffer _recvBuffer;
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
        resetDTSend();
        return _sendBuffer.writeToSocket(_sockfd);
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

    CellClient(SOCKET sockfd = INVALID_SOCKET) :
        _sendBuffer(SEND_BUFF_SIZE),
        _recvBuffer(RECV_BUFF_SIZE)
    {
        static int n = 1;
        id = n++;
        this->_sockfd = sockfd;

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

    int sendData(netmsg_DataHeader* header)
    {
        return sendData((const char*)header,header->dataLength);
    }

    int sendData(const char* pData,int len)
    {
        if(_sendBuffer.push(pData,len))
        {
            return len;
        }
        return -1;
    }

    int recvData()
    {
        return _recvBuffer.readFromSocket(_sockfd);
    }

    bool hasMsg()
    {
        return _recvBuffer.hasMsg();
    }

    netmsg_DataHeader* frontMsg()
    {
        return (netmsg_DataHeader*)_recvBuffer.data();
    }

    void popFrontMsg()
    {
        if(hasMsg())
        {
            _recvBuffer.pop(frontMsg()->dataLength);
        }
    }

    bool needWrite()
    {
        return _sendBuffer.needWrite();
    }
};

typedef  std::shared_ptr<CellClient> ClientSocketPtr;

#endif