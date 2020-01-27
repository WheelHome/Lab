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
#include <map>
#include <signal.h>
#include <memory>

#include "messageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "ClientSocket.hpp"
#include "CellSendMsgToClientTask.hpp"
#include "CellTask.hpp"

typedef   std::shared_ptr<CellSendMsgToClientTask> CellSendMsgToClientTaskPtr;

class CellServer;
//net event interface
class INetEvent 
{
private:

public:
    //Client quited event
    virtual void OnNetLeave(ClientSocketPtr& pClient) = 0;
    //Client msg event
    virtual void OnNetMsg(CellServer* pCellServer,ClientSocketPtr& pClient,DataHeader* header) = 0;
    //recv event
    virtual void OnNetRecv(ClientSocketPtr& pClient) = 0;
    //new client join event
    virtual void OnNetJoin(ClientSocketPtr& pClient) = 0;
};


class CellServer
{
private:
    SOCKET _sock;
    //formal client queue
	std::map<SOCKET,ClientSocketPtr> _clients;
    //clients buffer queue
    std::vector<ClientSocketPtr> _clientsBuff;
    //buffer queue lock
    std::mutex _mutex;
    std::thread* _pThread;
    //net event object
    INetEvent* _pNetEvent;

    CellTaskServer* _taskServer;
public:

    void addSendTask(ClientSocketPtr pClient,DataHeader* header)
    {
        auto  task = std::make_shared<CellSendMsgToClientTask>(pClient,header);
        _taskServer->addTask(task);
    }

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
        _taskServer = new CellTaskServer();
        _taskServer->Start();
    }

    //send data to single socket
    int sendData(SOCKET cSock,DataHeader* header)
    {
        if(isRun() && header)
        {
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGPIPE);
            sigprocmask(SIG_BLOCK, &set, NULL);
            send(cSock,(const char*)header,header->dataLength,0);
        }
        return SOCKET_ERROR;
    }

    void addClient(ClientSocketPtr pClient)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _clientsBuff.push_back(pClient);
    }

    CellServer(SOCKET sock = INVALID_SOCKET)
    {
        _sock = sock;
        _pThread = nullptr;
        _pNetEvent = nullptr;
        _taskServer = nullptr;
    }

    ~CellServer()
    {
        CLose();
        _sock = INVALID_SOCKET;
        if(_pThread!=nullptr)
        {
            delete _pThread;
        }
        if(_taskServer!=nullptr)
        {
            delete _taskServer;
        }
        _pThread = nullptr;
        _taskServer = nullptr;
    }

    //backup fdRead
    fd_set _fdRead_bak;
    //client list change
    bool _clients_change = false;
    SOCKET _maxSock; 
    //Handle net msg

    bool onRun()
    {
        while(isRun())
        {
            //From buffer to get cilent data
            _mutex.lock();
            if(!_clientsBuff.empty())
            {
                for(auto pClient : _clientsBuff)
                {
                    _clients[pClient->getSockfd()] = pClient;
                }
                _clientsBuff.clear();
                _clients_change = true;
            }
            _mutex.unlock();
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

            if(_clients_change)
            {
                _clients_change = false;
                _maxSock = _clients.begin()->second->getSockfd();
                for (auto iter : _clients)
                {
                    FD_SET(iter.second->getSockfd(), &fdRead);
                    if (_maxSock < iter.second->getSockfd())
                    {
                        _maxSock = iter.second->getSockfd();
                    }
                }
                memcpy(&_fdRead_bak,&fdRead,sizeof(fd_set));
            }
            else
            {
                memcpy(&fdRead,&_fdRead_bak,sizeof(fd_set));
            }
            //nfds
            timeval t = {0,10};
            int ret = select(_maxSock + 1,&fdRead,&fdWrite,&fdExp,&t);
            if(ret < 0)
            {
                std::cout << "Select error!" << std::endl;
                std::cout << errno << std::endl;
                CLose();
                return false;
            }
            else if (ret == 0)
            {
                continue;
            }
            
            #ifdef _WIN32
            for (int n = 0; n < fdRead.fd_count; n++)
            {
                auto iter  = _clients.find(fdRead.fd_array[n]);
                if (iter != _clients.end())
                {
                    if (-1 == RecvData(iter->second))
                    {
                        if (_pNetEvent)
                            _pNetEvent->OnNetLeave(iter->second);
                        _clients_change = true;
                        _clients.erase(iter->first);
                    }
                }else {
                    printf("error. if (iter != _clients.end())\n");
                }
            }
            #else
            
            std::vector<ClientSocketPtr> temp;
           for (auto& iter : _clients)
            {
                if (FD_ISSET(iter.first, &fdRead))
                {
                    if (-1 == recvData(iter.second))
                    {
                        if (_pNetEvent)
                            _pNetEvent->OnNetLeave(iter.second);
                        _clients_change = true;
                        _mutex.lock();
                        temp.push_back(iter.second);
                        _mutex.unlock();
                    }
                }  
            }
            for (auto pClient : temp)
            {
                _clients.erase(pClient->getSockfd());
                closesocket(pClient->getSockfd());
            }
            #endif
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
            #ifdef _WIN32
            for (auto iter : _clients)
            {
                closesocket(iter.second->sockfd());
                delete iter.second;
            }
            //关闭套节字closesocket
            closesocket(_sock);
            #else
            for(auto iter : _clientsBuff)
            {
                closesocket(iter->getSockfd());
            }
            for (auto iter : _clients)
            {
                closesocket(iter.second->getSockfd());
            }
            //关闭套节字closesocket
            closesocket(_sock);
            #endif
        }
        _sock = INVALID_SOCKET;
        #ifdef _WIN32
        WSACleanup();
        #endif
        _clients.clear();
        _clientsBuff.clear();
    }    
    
    int recvData(ClientSocketPtr pClient)
    {
        char* szRecv = pClient->getMsgBuf() + pClient->getLastPos();
        int nLen = recv(pClient->getSockfd(),szRecv,RECV_BUFF_SIZE - pClient->getLastPos(),0);
        _pNetEvent->OnNetRecv(pClient);
        if(nLen <= 0)
        {
            //std::cout << "Client socket = " << pClient->getSockfd() << " quited." << std::endl;
            return -1;
        }
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
    virtual void onNetMsg(ClientSocketPtr pClient,DataHeader* header)
    {
        _pNetEvent->OnNetMsg(this,pClient,header);
    }
};

typedef std::shared_ptr<CellServer> CellServerPtr;

class EasyTcpServer :public INetEvent
{
private:
    SOCKET _sock;
    //msg dealed object
    std::vector<CellServerPtr> _cellServers;
    //Time count
    CELLTimestamp _tTime;
protected:
    //received msg packages
    std::atomic_int _recvCount;
    std::atomic_int _clientCount;
    std::atomic_int _msgCount;
public:

    EasyTcpServer()
    {
        _sock = INVALID_SOCKET;
        _recvCount = 0;
        _clientCount = 0;
        _msgCount = 0;
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
            addClientToCellServer(std::make_shared<ClientSocket>(cSock));
        }
        return cSock;
    }

    void addClientToCellServer(ClientSocketPtr pClient)
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
            auto ser = std::make_shared<CellServer>(_sock);
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
        _recvCount = 0;
        _clientCount = 0;
        _msgCount = 0;
        _cellServers.clear();
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
            std::cout << "thread="<< _cellServers.size()<< " time = " << t1 << " clients=" <<  _clientCount << " Received package recvCount = " << (int)_recvCount / t1 << " msgCount=" << (int)_msgCount / t1 << std::endl;
            _recvCount = 0;
            _msgCount = 0;
            _tTime.update();
        }
    }

    virtual void OnNetLeave(ClientSocketPtr& pClient)
    {
        _clientCount--;
    }

    virtual void OnNetMsg(CellServer* pCellServer,ClientSocketPtr& pClient,DataHeader* header)
    {
        _msgCount++;
    }

    virtual void OnNetJoin(ClientSocketPtr& pClient)
    {
        _clientCount++;
  
    }

    virtual void OnNetRecv(ClientSocketPtr& pClient)
    {
        _recvCount++;
    }
};

#endif