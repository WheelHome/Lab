#ifndef _EASYTCPSERVER_HPP_
#define _EASYTCPSERVER_HPP_

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

#include <vector>
#include <string.h>
#include <iostream>
#include "messageHeader.hpp"

//Buf minimum size 
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif


class ClientSocket
{
private:
    SOCKET _sockfd;//socket fd_set desc set
    //msgBuf
    char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
    //The msgBuf end
    long unsigned int _lastPos = 0;
public:
    ClientSocket(SOCKET _sockfd = INVALID_SOCKET)
    {
        this->_sockfd = _sockfd;
        bzero(_szMsgBuf,sizeof(_szMsgBuf));
        this->_lastPos = 0;
    }

    SOCKET getSockfd()
    {
        return _sockfd;
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
};

class EasyTcpServer
{
private:
    SOCKET _sock;
    std::vector<ClientSocket*> _clients;
public:
    EasyTcpServer()
    {
        _sock = INVALID_SOCKET;
    }

    ~EasyTcpServer()
    {
        if(_sock != INVALID_SOCKET){
            this->CLose();
        }
        _sock = INVALID_SOCKET;
    }

    //Init server socket
    SOCKET initSocket()
    {
        #ifdef _WIN32
        WORD ver = MAKEWORD(2,2);
        WSADATA dat;
        WSAStartup(ver,&dat);
        #endif
        if(_sock != INVALID_SOCKET)
        {
            std::cout << "Close existed socket=" << _sock << std::endl;
            this->CLose();
        }
        _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_sock == INVALID_SOCKET)
        {
            std::cout << "Create socket  error!" << std::endl;
            return -1;
        }
        return _sock;
    }

    //BInd server socket
    int Bind(const char* ip,unsigned short port)
    {
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
        auto ipAddr = ip?inet_addr(ip):INADDR_ANY;

        #ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = ipAddr;
        #else
            _sin.sin_addr.s_addr = ipAddr;
        #endif

        int ret = bind(_sock,(const sockaddr*)&_sin,sizeof(sockaddr_in));
        if(ret == SOCKET_ERROR)
        {
            std::cout << "Bind Socket=" << _sock << " Error!" << std::endl;
            return -1;
        }
        else
        {
            std::cout <<"Bind Socket:" << _sock << " success!" << std::endl;
        }
        return ret;
    }

    //Listen server socket
    int Listen(int n)
    {
        int ret = listen(_sock,n);
        if(ret == SOCKET_ERROR)
        {
            std::cout << "Listen Socket Error!" << std::endl;
            return -1;
        }
        else
        {
            std::cout <<"Listen Socket:" << _sock << " success!" << std::endl;
        }
        return ret;
    }

    //Accept client socket
    SOCKET Accept()
    {
        sockaddr_in clientAddr = {};

        int nAddrLen = sizeof(clientAddr);
        SOCKET cSock = INVALID_SOCKET;

        #ifdef _WIN32
        cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddrLen);
        #else
        cSock = accept(_sock,(sockaddr*)&clientAddr,(socklen_t*)&nAddrLen);
        #endif

        if(cSock == INVALID_SOCKET)
        {
            std::cout << "Received wrong client socket=" << cSock  << std::endl;
            return -1;
        }
        else
        {            
            NewUserJoin userJoin = {};
            sendDataToAll(&userJoin);
            std::cout << "New client socket = " << cSock << " joined in and it's IP=" << inet_ntoa(clientAddr.sin_addr)<< std::endl;
            _clients.push_back(new ClientSocket(cSock));
        }
        return cSock;
    }

    //Close socket
    void CLose()
    {
        if(_sock != INVALID_SOCKET)
        {
            for(size_t n = 0; n < _clients.size(); ++n)
            {
                closesocket(_clients[n]->getSockfd());
                delete _clients[n];
            }
            closesocket(_sock);

            #ifdef _WIN32
            WSACleanup();
            #endif
        }
        _sock = INVALID_SOCKET;
        #ifdef _WIN32
        WSACleanup();
        #endif
        _clients.clear();
    }

    //Handle net msg
    bool onRun()
    {
        if(isRun())
        {
            // socket
            fd_set fdRead;
            fd_set fdWrite;
            fd_set fdExp;

            FD_ZERO(&fdRead);
            FD_ZERO(&fdWrite);
            FD_ZERO(&fdExp);

            FD_SET(_sock,&fdRead);
            FD_SET(_sock,&fdWrite);
            FD_SET(_sock,&fdExp);
            SOCKET maxSock = _sock;
            for(unsigned long int  n = 0; n < _clients.size(); ++n)
            {
                FD_SET(_clients[n]->getSockfd(),&fdRead);
                if(maxSock < _clients[n]->getSockfd())
                {
                    maxSock = _clients[n]->getSockfd();
                }
            }
            //nfds

            timeval t = {1,0};
            int ret = select(maxSock + 1,&fdRead,&fdWrite,&fdExp,&t);
            if(ret < 0)
            {
                std::cout << "select error!" << std::endl;
                CLose();
                return false;
            }
            if(FD_ISSET(_sock,&fdRead))
            {
                FD_CLR(_sock,&fdRead);
                Accept();
            }
            for(unsigned long int n = 0; n < _clients.size(); ++n)
            {
                if(FD_ISSET(_clients[n]->getSockfd(),&fdRead))
                {
                    if(recvData(_clients[n]) == -1)
                    {
                        auto iter = _clients.begin() + n;
                        if(iter != _clients.end())
                        {
                            _clients.erase(iter);
                            closesocket(_clients[n]->getSockfd());
                            delete _clients[n];
                        }
                    }
                }
            }
            return true;
        }
        return true;
    }

    //isRun
    bool isRun()
    {
        return _sock != INVALID_SOCKET;
    }

    //DataHeaderBuffer
    char _szRecv[RECV_BUFF_SIZE] = {};
    int recvData(ClientSocket* pClient)
    {
        int nLen = recv(pClient->getSockfd(),_szRecv,RECV_BUFF_SIZE,0);
        //std::cout << "Received command  " << _recvBuf << std::cout;
        if(nLen <= 0)
        {
            std::cout << "Client socket = " << pClient->getSockfd() << " quited." << std::endl;
            return -1;
        }
        //Move received data to msgBuf
        memcpy(pClient->getMsgBuf() + pClient->getLastPos(),_szRecv,nLen);
        //Move msgBuf pointer to it's tail
        pClient->setLastPos(pClient->getLastPos() + nLen) ;
        //Received a integrated msgHeader
        while(pClient->getLastPos()  >= sizeof(DataHeader))
        {
            //There can be know the all msgData
            DataHeader* header = (DataHeader*)pClient->getMsgBuf();
            //Received a integrated msgData
            if((int)pClient->getLastPos() > header->dataLength)
            {
                //The msgData length  is waitting to handle in msgBuf
                int nSize = pClient->getLastPos() - header->dataLength;
                //Deal with net msg
                onNetMsg(pClient->getSockfd(),header);
                //Move the untrated msgData to begin
                memcpy(pClient->getMsgBuf() ,pClient->getMsgBuf()  + header->dataLength,pClient->getLastPos() - header->dataLength);
                //Pointer move to begin
                pClient->setLastPos(nSize);
            }
            else
            {
                //Untreated msgData isn't a integrated msg.
                return 1;
            }
        }
        /*
        recv(cSock,szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        onNetMsg(cSock,header);*/
        return 1;
    }

    //response net msg
    virtual void onNetMsg(SOCKET cSock,DataHeader* header)
    {
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            Login* login = (Login*)header;
            LoginResult ret;
            std::cout << "Received command CMD_LOGIN" << " dataLength:" << header->dataLength << " userName:" <<
                    login->userName << " userPassword:" << login->passWord<< std::endl;
            sendData(cSock,&ret);
            break;
        }
        case CMD_LOGOUT:
        {
            Logout* logout = (Logout*)header;
            std::cout << "Received command CMD_LOGOUT" << " dataLength:" << logout->dataLength << " userName:" <<
                    logout->userName << std::endl;
            LogoutResult ret;
            //send(cSock,(const char*)&header,sizeof(DataHeader),0);
            sendData(cSock,&ret);
            break;
        }
        case CMD_ERROR:
        {
            std::cout << "Received command CMD_ERROR" << " dataLength:"  << header->dataLength <<  std::endl;
        }
        default:
        {
            std::cout << "Received undefine msg" << " dataLength:" << header->dataLength <<  std::endl;
            DataHeader eheader ;
            sendData(cSock,&eheader);
        }
        }
    }

    //send data to single socket
    int sendData(SOCKET cSock,DataHeader* header)
    {
        if(isRun() && header)
        {
            send(cSock,(const char*)header,header->dataLength,0);
        }
        return SOCKET_ERROR;
    }

    //send data to all existed socket
    void sendDataToAll(DataHeader* header)
    {
        if(isRun() && header)
        {
            for(unsigned long int n = 0; n < _clients.size(); ++n)
            {
                sendData(_clients[n]->getSockfd(),header);
            }
        }
    }
};

#endif