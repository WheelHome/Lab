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

class EasyTcpClient
{
private:
    SOCKET _sock;
    bool _isConnect;
public:
    EasyTcpClient()
    {
        _sock = INVALID_SOCKET;
        _isConnect = false;
    }
    virtual  ~EasyTcpClient()
    {
        this->CLose();
    }
    //Init CellClient
    int initSocket()
    {
        #ifdef _WIN32
        WORD ver = MAKEWORD(2,2);
        WSADATA dat;
        WSAStartup(ver,&dat);
        #endif
        if(_sock != INVALID_SOCKET)
        {
            std::cout << "Close exited socket!" << std::endl;
            this->CLose();
        }
        _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_sock == INVALID_SOCKET)
        {
            std::cout << "Create socket  error!" << std::endl;
            return -1;
        }
        return 1;
    }

    //Connect server
    int Connect(char* ip,short port)
    {
        if(_sock == INVALID_SOCKET)
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

        int ret = connect(_sock,(const sockaddr*)&_sin,sizeof(sockaddr_in));

        if( ret == SOCKET_ERROR)
        {
            std::cout << "Connect socket error!" << std::endl;
            _isConnect = false;
        }
        _isConnect = true;
        return ret;
    }

    //close CellClient
    void CLose()
    {
        if(_sock != INVALID_SOCKET)
        {
            closesocket(_sock);
            
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
        _sock = INVALID_SOCKET;
        _isConnect = false;
    }

    //Handle net msg
    bool onRun()
    {
        if(isRun())
        {
            fd_set fdReads;
            FD_ZERO(&fdReads);
            FD_SET(_sock,&fdReads);
            timeval t = {0,10};
            int ret = select(_sock + 1,&fdReads,nullptr,nullptr,&t);
            if(ret < 0)
            {
                std::cout << "Socket="<< _sock <<" select completed!" << std::endl;
                return false;
            }
            if(FD_ISSET(_sock,&fdReads))
            {
                FD_CLR(_sock,&fdReads);
                if(recvData(_sock) == -1)
                {
                    std::cout << "Socket="<< _sock << " Select finished" << std::endl;
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool isRun()
    {
        return _sock != INVALID_SOCKET && _isConnect;
    }

    //response net data
    virtual void onNetMsg(netmsg_DataHeader* header)
    {
        switch (header->cmd)
        {
        case CMD_LOGIN_RESULT:
        {
            //std::cout << "Received command CMD_LOGIN_RESULT"  << " dataLength:" << header->dataLength << std::endl;
            break;
        }
        case CMD_LOGOUT_RESULT:
        {
            break;
        }
        case CMD_NEW_USER_JOIN:
        {
            break;
        }
        default:
        {
        }
        }
    }

    //Buf minimum size 
    #ifndef RECV_BUFF_SIZE
    #define RECV_BUFF_SIZE 10240
    #endif
    //msgBuf
    char _szMsgBuf[RECV_BUFF_SIZE * 5] = {};
    //The msgBuf end
    int _lastPos = 0;
    //received data
    //receive data from server
    int recvData(SOCKET _cSock)
    {
        char* szRecv = _szMsgBuf + _lastPos;
        int nLen = recv(_cSock,szRecv,(RECV_BUFF_SIZE * 5) - _lastPos ,0);
        if(nLen <= 0)
        {
            std::cout << "Connection broken" << std::endl;
            return -1;
        }
        //Move msgBuf pointer to it's tail
        _lastPos += nLen;
        //Received a integrated msgHeader
        while(_lastPos >= sizeof(netmsg_DataHeader))
        {
            //There can be know the all msgData
            netmsg_DataHeader* header = (netmsg_DataHeader*)_szMsgBuf;
            //Received a integrated msgData
            if(_lastPos > header->dataLength)
            {
                //The msgData length  is waitting to handle in msgBuf
                int nSize = _lastPos - header->dataLength;
                //Deal with net msg
                onNetMsg(header);
                //Move the untrated msgData to begin
                memcpy(_szMsgBuf,_szMsgBuf + header->dataLength,_lastPos - header->dataLength);
                //Pointer move to begin
                _lastPos = nSize;
            }
            else
            {
                //Untreated msgData isn't a integrated msg.
                return 1;
            }
        }
        return 1;
    }

    //send Data to server
    int sendData(netmsg_DataHeader* header)
    {
        int ret = -1;
        if(isRun() && header)
        {
            ret = send(_sock,(const char*)header,header->dataLength,0);
            if(SOCKET_ERROR == ret){
                CLose();
            }
        }
        return ret;
    }
};

#endif