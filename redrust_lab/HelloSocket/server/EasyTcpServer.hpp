#ifndef _EASYTCPSERVER_HPP_
#define _EASYTCPSERVER_HPP_

#include "Cell.hpp"
#include "messageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CellSemaphore.hpp"
#include "CellClient.hpp"
#include "CellTask.hpp"
#include "INetEvent.hpp"
#include "CellServer.hpp"

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
            ClientSocketPtr c(new CellClient(cSock));
            addClientToCellServer(c);
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
    }

    //launch CellServer
    void Start(int nCellServer)
    {
        for(int n = 0; n < nCellServer; n++)
        {
            auto ser = std::make_shared<CellServer>(n + 1);
            _cellServers.push_back(ser);
            //registed msg event object
            ser->setEventObj(this);
            ser->Start();
        }
    }

    //Close socket
    void CLose()
    {
        std::cout << "EasyTcpServer.Close() 1" << std::endl;
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
        std::cout << "EasyTcpServer.Close() 2" << std::endl;
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

    virtual void OnNetMsg(CellServer* pCellServer,ClientSocketPtr& pClient,netmsg_DataHeader* header)
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