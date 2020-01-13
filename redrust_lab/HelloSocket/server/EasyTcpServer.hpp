#ifndef _EASYTCPSERVER_HPP_
#define _EASYTCPSERVER_HPP_

#ifdef _WIN32
#define FD_SETSIZE 1024
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
#include <thread>
#include <mutex>
#include <atomic>
#include "messageHeader.hpp"
#include "CELLTimestamp.hpp"

//Buf minimum size 
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

//Client data type
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

    int sendData(DataHeader* header)
    {
        if( header)
        {
            send(_sockfd,(const char*)header,header->dataLength,0);
        }
        return SOCKET_ERROR;
    }
};

//net event interface
class INetEvent 
{
private:

public:
    //Client quited event
    virtual void OnNetLeave(ClientSocket* pClient) = 0;
    //Client msg event
    virtual void OnNetMsg(ClientSocket* pClient,DataHeader* header) = 0;
    //new client join event
    virtual void OnNetJoin(ClientSocket* pClient) = 0;
};

class CellServer
{
private:
    SOCKET _sock;
    //formal client queue
    std::vector<ClientSocket*> _clients;
    //clients buffer queue
    std::vector<ClientSocket*> _clientsBuff;
    //buffer queue lock
    std::mutex _mutex;
    std::thread* _pThread;
    //net event object
    INetEvent* _pNetEvent;
public:

    void setEventObj(INetEvent* event)
    {
        _pNetEvent = event;
    }

    size_t getClientCount()
    {
        return _clients.size() + _clientsBuff.size();
    }

    void Start()
    {
        _pThread = new std::thread(std::mem_fun(&CellServer::onRun),this);

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

    void addClient(ClientSocket* pClient)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _clientsBuff.push_back(pClient);
    }

    CellServer(SOCKET sock = INVALID_SOCKET)
    {
        _sock = sock;
        _pThread = nullptr;
        _pNetEvent = nullptr;
    }

    ~CellServer()
    {
        CLose();
        _sock = INVALID_SOCKET;
        if(_pThread!=nullptr)
        {
            delete _pThread;
        }
        _pThread = nullptr;
    }

    //Handle net msg
    bool onRun()
    {
        while(isRun())
        {
            //From buffer to get cilent data
            if(_clientsBuff.size() > 0)
            {
                std::lock_guard<std::mutex> lock(_mutex);
                for(auto pClient : _clientsBuff)
                {
                    _clients.push_back(pClient);
                }
                _clientsBuff.clear();
            } 

            //if there not have client
            if(_clients.empty())
            {
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
                continue;
            }
            // socket
            fd_set fdRead;
            fd_set fdWrite;
            fd_set fdExp;

            FD_ZERO(&fdRead);
            FD_ZERO(&fdWrite);
            FD_ZERO(&fdExp);

            SOCKET maxSock = _clients[0]->getSockfd();
            for(unsigned long int  n = 0; n < _clients.size(); ++n)
            {
                FD_SET(_clients[n]->getSockfd(),&fdRead);
                if(maxSock < _clients[n]->getSockfd())
                {
                    maxSock = _clients[n]->getSockfd();
                }
            }
            //nfds
            timeval t = {0,10};
            int ret = select(maxSock + 1,&fdRead,&fdWrite,&fdExp,&t);
            if(ret < 0)
            {
                std::cout << "Select error!" << std::endl;
                std::cout << errno << std::endl;
                CLose();
                return false;
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
                            if(_pNetEvent)
                            {
                                _pNetEvent->OnNetLeave(_clients[n]);
                            }
                            //closesocket(_clients[n]->getSockfd());
                            delete _clients[n];
                            _clients.erase(iter);
                        }
                    }
                }
            }
        }
        return true;
    }

    //isRun
    bool isRun()
    {
        return _sock != INVALID_SOCKET;
    }

    //Close socket
    void CLose()
    {
        if(_sock != INVALID_SOCKET)
        {
            for(size_t n = 0; n < _clients.size(); ++n)
            {
                if(_clients[n] != nullptr)
                {
                    closesocket(_clients[n]->getSockfd());
                    delete _clients[n];
                }
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
    
    char _szRecv[RECV_BUFF_SIZE] = {};
    int recvData(ClientSocket* pClient)
    {
        int nLen = recv(pClient->getSockfd(),_szRecv,RECV_BUFF_SIZE,0);
        if(nLen <= 0)
        {
            //std::cout << "Client socket = " << pClient->getSockfd() << " quited." << std::endl;
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
                onNetMsg(pClient,header);
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
        return 1;
    }
    //response net msg
    virtual void onNetMsg(ClientSocket* pClient,DataHeader* header)
    {
        _pNetEvent->OnNetMsg(pClient,header);
    }
};

class EasyTcpServer :public INetEvent
{
private:
    SOCKET _sock;
    //msg dealed object
    std::vector<CellServer*> _cellServers;
    //Time count
    CELLTimestamp _tTime;
public:
    //received msg packages
    std::atomic_int _recvCount;
    std::atomic_int _clientCount;
public:

    EasyTcpServer()
    {
        _sock = INVALID_SOCKET;
        _recvCount = 0;
        _clientCount = 0;
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
            addClientToCellServer(new ClientSocket(cSock));
        }
        return cSock;
    }

    void addClientToCellServer(ClientSocket* pClient)
    {
        auto pMinServer = _cellServers[0];
        //Search the cellserver that having minimum clients 
        for(auto pCellServer : _cellServers)
        {
            if(pMinServer->getClientCount() > pCellServer->getClientCount())
            {
                pMinServer = pCellServer;
            }
        }
        pMinServer->addClient(pClient);
        OnNetJoin(pClient);
    }

    //launch CellServer
    void Start(int nCellServer)
    {
        for(int n = 0; n < nCellServer; n++)
        {
            auto ser = new CellServer(_sock);
            _cellServers.push_back(ser);
            //registed msg event object
            ser->setEventObj(this);
            ser->Start();
        }
    }

    //Close socket
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
        #ifdef _WIN32
        WSACleanup();
        #endif
    }

    //Handle net msg
    bool onRun()
    {
        if(isRun())
        {
            timeToMsg();
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
            //nfds

            timeval t = {0,10};
            int ret = select(_sock + 1,&fdRead,&fdWrite,&fdExp,&t);
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
        }
        return true;
    }

    //isRun
    bool isRun()
    {
        return _sock != INVALID_SOCKET;
    }

    //response net msg
    void timeToMsg()
    {
        auto t1 = _tTime.getEpalsedSecond();
        if( t1 >= 1.0)
        {
            std::cout << "time = " << t1 << " clients=" <<  _clientCount << " Received package recvCount = " << (int)_recvCount / t1 << std::endl;
            _recvCount = 0;
            _tTime.update();
        }
    }

    virtual void OnNetLeave(ClientSocket* pClient)
    {
        _clientCount--;
    }

    virtual void OnNetMsg(ClientSocket* pClient,DataHeader* header)
    {
        _recvCount++;
    }

    virtual void OnNetJoin(ClientSocket* pClient)
    {
        _clientCount++;
    }
};

#endif