#ifndef __CELLSERVER__H__
#define __CELLSERVER__H__

#include "Cell.hpp"
class INetEvent;
class CellServer
{
private:
    //formal client queue
	std::map<SOCKET,ClientSocketPtr> _clients;
    //clients buffer queue
    std::vector<ClientSocketPtr> _clientsBuff;
    //buffer queue lock
    std::mutex _mutex;
    //net event object
    INetEvent* _pNetEvent;

    CellTaskServer  _taskServer;
    CellSemaphore _sem;

    //backup fdRead
    fd_set _fdRead_bak;

    time_t _old_time = CELLTime::getNowInMilliSec();
    int _id = -1;
    //client list change
    bool _clients_change = true;
    bool _isRun;
    SOCKET _maxSock; 

private:
    void ClearClients()
    {
        _clients.clear();
        _clientsBuff.clear();
    }

public:

    void addSendTask(ClientSocketPtr pClient,netmsg_DataHeader* header)
    {
        /*auto  task = std::make_shared<CellSendMsgToClientTask>(pClient,header);
        auto temp = std::dynamic_pointer_cast<CellTask>(task);
        _taskServer.addTask(temp);*/
        
       _taskServer.addTask([pClient,header](){
           pClient->sendData(header);
       });
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
        if(!_isRun)
        {
            _isRun = true;
            std::thread pThread = std::thread(std::mem_fun(&CellServer::onRun),this);
            pThread.detach();
            _taskServer.Start();
        }
    }

    //send data to single socket
    int sendData(SOCKET cSock,netmsg_DataHeader* header)
    {
        if(_isRun && header)
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

    CellServer(int id)
    {
        _id = id;
        _pNetEvent = nullptr;
        _taskServer._serverId = id;
    }

    ~CellServer()
    {
        CLose();
    }

 
    //Handle net msg
    bool onRun()
    {
        while(_isRun)
        {
            //From buffer to get cilent data
            _mutex.lock();
            if(!_clientsBuff.empty())
            {
                for(auto pClient : _clientsBuff)
                {
                    _clients[pClient->getSockfd()] = pClient;
                    pClient->_serverId = _id;
                    if(_pNetEvent)
                    {
                        _pNetEvent->OnNetJoin(pClient);
                    }
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
                _old_time = CELLTime::getNowInMilliSec();
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
            /*
            else if (ret == 0)
            {
                continue;
            }*/
            
            ReadData(fdRead);
         //   CheckTime();
        }
        this->ClearClients();
        _sem.wakeup();
        return true;
    }

    void CheckTime()
    {
        auto tNow = CELLTime::getNowInMilliSec();
        auto dt = tNow - _old_time;
        _old_time = tNow;
        for(auto iter = _clients.begin(); iter != _clients.end();)
        {
            //Heart beat check
            if(iter->second->checkHeart(dt))
            {
                if (_pNetEvent)
                    _pNetEvent->OnNetLeave(iter->second);
                _clients_change = true;
                _mutex.lock();
                auto iterOld = iter++;
                _clients.erase(iterOld);
                _mutex.unlock();
                //closesocket(iterOld->second->getSockfd());
            }else
            {
                iter->second->checkSend(dt);
                iter++;
            }
        }
    }

    void ReadData(fd_set& fdRead)
    {
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
                    //closesocket(iter->first);
                    delete iter->second;
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
                    temp.push_back(iter.second);
                }
            }  
        }
        for (auto pClient : temp)
        {
            _clients.erase(pClient->getSockfd());
         //   closesocket(pClient->getSockfd());
        }
        #endif
    }

    //Close socket
    void CLose()
    {
        if(_isRun)
        {
            std::cout << "CellServer:" << _id << ".Close() 1" << std::endl;
            _taskServer.Close();
            _isRun = false;
            _sem.wait();
            std::cout << "CellServer:" << _id << ".Close() 2" << std::endl;
        }
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

        pClient->resetDTHeart();
        //Move msgBuf pointer to it's tail
        pClient->setLastPos(pClient->getLastPos() + nLen) ;
        //Received a integrated msgHeader
        while(pClient->getLastPos()  >= sizeof(netmsg_DataHeader))
        {
            //There can be know the all msgData
            netmsg_DataHeader* header = (netmsg_DataHeader*)pClient->getMsgBuf();
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
    virtual void onNetMsg(ClientSocketPtr pClient,netmsg_DataHeader* header)
    {
        _pNetEvent->OnNetMsg(this,pClient,header);
    }
};
#endif  //!__CELLSERVER__H__