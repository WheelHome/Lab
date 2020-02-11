#ifndef _EASYTCPCLIENT_HPP_
#define _EASYTCPCLIENT_HPP_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#define SOCKET int
#define closesocket close
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#endif

#include <string.h>
#include <thread>
#include <iostream>
#include <vector>

#include "CellNetWork.hpp"
#include "CellClient.hpp"

class EasyTcpClient
{
protected:
    CellClient* pClient = nullptr;
    bool _isConnect = false;
    std::mutex _mutex;
public:
    EasyTcpClient()
    {
        _isConnect = false;
    }

    virtual  ~EasyTcpClient()
    {
        this->CLose();
    }

    //Init CellClient
    int initSocket()
    {
        CellNetWork::Instance();
        if(pClient)
        {
            CLose();
        }
        int _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_sock == INVALID_SOCKET)
        {
            std::cout << "Create socket  error!" << std::endl;
            return -1;
        }
        pClient = new CellClient(_sock);
        return 1;
    }

    //Connect server
    int Connect(char* ip,short port)
    {
        if(!pClient)
        {
            initSocket();
        }
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);

        #ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
        #else
        _sin.sin_addr.s_addr = inet_addr(ip);
        #endif

        int ret = connect(pClient->getSockfd(),(const sockaddr*)&_sin,sizeof(sockaddr_in));

        if( ret == SOCKET_ERROR)
        {
            std::cout << "Connect socket error!" << std::endl;
            _isConnect = false;
            return ret;
        }
        _isConnect = true;
        return ret;
    }

    //close CellClient
    void CLose()
    {
        if(pClient)
        {
            delete pClient;
            pClient = nullptr;
        }
        _isConnect = false;
    }

    //Handle net msg
    bool onRun()
    {
        if(isRun())
        {
            SOCKET _sock = pClient->getSockfd();
            fd_set fdReads;
            fd_set fdWrite;
            FD_ZERO(&fdReads);
            FD_SET(_sock,&fdReads);
            FD_ZERO(&fdWrite);
            FD_SET(_sock,&fdWrite);
            timeval t = {0,1};
            int ret = select( _sock + 1,&fdReads,&fdWrite,nullptr,&t);
            if(ret < 0)
            {
                std::cout << "Socket="<< _sock <<" select completed!" << std::endl;
                return false;
            }
            if(FD_ISSET(_sock,&fdReads))
            {
                if(-1 == recvData())
                {
                    std::cout << "Socket="<< _sock << " Select read finished" << std::endl;
                    return false;
                }
            }
            if(FD_ISSET(_sock,&fdWrite))
            {
                if(-1 == pClient->sendDataImme())
                {
                    std::cout << errno << std::endl;
                    std::cout << "Socket="<< _sock << " Select write finished" << std::endl;
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool isRun()
    {
        return pClient && _isConnect;
    }

    //response net data
    virtual void onNetMsg(netmsg_DataHeader* header) = 0;

    //receive data from server
    int recvData()
    {
        if(isRun())
        {
            int nLen = pClient->recvData();
            if(nLen > 0)
            {
                while(pClient->hasMsg())
                {
                    onNetMsg(pClient->frontMsg());
                    pClient->popFrontMsg();  
                }
            }
            return nLen;
        }
        return 0;
    }

    //send Data to server
    int sendData(netmsg_DataHeader* header)
    {
        if(isRun())
            return pClient->sendData(header);
        return 0;
    }   
    int sendData(const char* pData,int len)
    {
        if(isRun())
            return pClient->sendData(pData,len);
        return 0;
    }
};

#endif