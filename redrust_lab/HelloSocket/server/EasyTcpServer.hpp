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

class EasyTcpServer
{
private:
    SOCKET _sock;
    std::vector<SOCKET> g_clients;
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
        SOCKET _cSock = INVALID_SOCKET;

        #ifdef _WIN32
        _cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddrLen);
        #else
        _cSock = accept(_sock,(sockaddr*)&clientAddr,(socklen_t*)&nAddrLen);
        #endif

        if(_cSock == INVALID_SOCKET)
        {
            std::cout << "Received wrong client socket=" << _cSock  << std::endl;
            return -1;
        }
        else
        {            
            NewUserJoin userJoin = {};
            sendDataToAll(&userJoin);
            std::cout << "New client socket = " << _cSock << " joined in and it's IP=" << inet_ntoa(clientAddr.sin_addr)<< std::endl;
            g_clients.push_back(_cSock);
        }
        return _cSock;
    }

    //Close socket
    void CLose()
    {
        if(_sock != INVALID_SOCKET)
        {
            for(size_t n = 0; n < g_clients.size(); ++n)
            {
                closesocket(g_clients[n]);
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
            for(unsigned long int  n = 0; n < g_clients.size(); ++n)
            {
                FD_SET(g_clients[n],&fdRead);
                if(maxSock < g_clients[n])
                {
                    maxSock = g_clients[n];
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
            for(unsigned long int n = 0; n < g_clients.size(); ++n)
            {
                if(FD_ISSET(g_clients[n],&fdRead))
                {
                    if(recvData(g_clients[n]) == -1)
                    {
                        auto iter = g_clients.begin() + n;
                        if(iter != g_clients.end())
                        {
                            g_clients.erase(iter);
                            closesocket(g_clients[n]);
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

    //received data
    int recvData(SOCKET _cSock)
    {
        //DataHeaderBuffer
        char szRecv[1024] = {};
        int nLen = recv(_cSock,szRecv,sizeof(DataHeader),0);
        DataHeader* header = (DataHeader*)szRecv;
        //std::cout << "Received command  " << _recvBuf << std::cout;
        if(nLen <= 0)
        {
            std::cout << "Client socket = " << _cSock << " quited." << std::endl;
            return -1;
        }
        recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        onNetMsg(_cSock,header);
        return 1;
    }

    //response net msg
    virtual void onNetMsg(SOCKET _cSock,DataHeader* header)
    {
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            Login* login = (Login*)header;
            LoginResult ret;
            std::cout << "Received command CMD_LOGIN" << " dataLength:" << login->dataLength << " userName:" <<
                    login->userName << " userPassword:" << login->passWord<< std::endl;
            send(_cSock,(const char*)&ret,sizeof(LoginResult),0);
            break;
        }
        case CMD_LOGOUT:
        {
            Logout* logout = (Logout*)header;
            std::cout << "Received command CMD_LOGOUT" << " dataLength:" << logout->dataLength << " userName:" <<
                    logout->userName << std::endl;
            LogoutResult ret;
            //send(_cSock,(const char*)&header,sizeof(DataHeader),0);
            send(_cSock,(const char*)&ret,sizeof(LogoutResult),0);
            break;
        }
        default:
        {
            DataHeader header = {0,CMD_ERROR};
            send(_cSock,(const char*)&header,sizeof(DataHeader),0);
        }
        }
    }

    //send data to single socket
    int sendData(SOCKET _cSock,DataHeader* header)
    {
        if(isRun() && header)
        {
            send(_cSock,(const char*)header,header->dataLength,0);
        }
        return SOCKET_ERROR;
    }

    //send data to all existed socket
    void sendDataToAll(DataHeader* header)
    {
        if(isRun() && header)
        {
            for(unsigned long int n = 0; n < g_clients.size(); ++n)
            {
                sendData(g_clients[n],header);
            }
        }
    }
};

#endif