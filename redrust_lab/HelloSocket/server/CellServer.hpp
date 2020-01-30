#ifndef __CELLSERVER__H__
#define __CELLSERVER__H__

#include "Cell.hpp"
class INetEvent;
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
    std::thread _pThread;
    //net event object
    INetEvent* _pNetEvent;

    CellTaskServer  _taskServer;
public:

    void addSendTask(ClientSocketPtr pClient,DataHeader* header)
    {
        auto  task = std::make_shared<CellSendMsgToClientTask>(pClient,header);
        auto temp = std::dynamic_pointer_cast<CellTask>(task);
        _taskServer.addTask(temp);
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
        _pThread = std::thread(std::mem_fun(&CellServer::onRun),this);
        _taskServer.Start();
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
        _pNetEvent = nullptr;
    }

    ~CellServer()
    {
        CLose();
        _sock = INVALID_SOCKET;
        if(_pThread.joinable())
        {
            _pThread.join();
        }
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
#endif  //!__CELLSERVER__H__