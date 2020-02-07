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
    CellThread _thread;
    
    //backup fdRead
    fd_set _fdRead_bak;

    time_t _old_time = CELLTime::getNowInMilliSec();
    int _id = -1;
    //client list change
    bool _clients_change = true;

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
        _taskServer.Start();
        _thread.Start(nullptr,
        [this](CellThread& pThread){
            onRun(pThread);
        },
        [this](CellThread& pThread){
           this->ClearClients();
        }  
        );
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
    void onRun(CellThread& pThread)
    {
        while(pThread.isRun())
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
            memcpy(&fdWrite,&_fdRead_bak,sizeof(fd_set));
            memcpy(&fdExp,&_fdRead_bak,sizeof(fd_set));
            //nfds
            timeval t = {0,10};
            int ret = select(_maxSock + 1,&fdRead,&fdWrite,&fdExp,&t);
            if(ret < 0)
            {
                std::cout << "CellServer " << _id << "onRun Select error exit!" << std::endl;
                std::cout << errno << std::endl;
                pThread.Exit();
                break;
            }
            /*
            else if (ret == 0)
            {
                continue;
            }*/
            ReadData(fdRead);
            WriteData(fdWrite);
         //   WriteData(fdExp);
            CheckTime();
        }
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
                auto iterOld = iter++;
                _clients.erase(iterOld);
                //closesocket(iterOld->second->getSockfd());
            }else
            {
                // iter->second->checkSend(dt);
                iter++;
            }
        }
    }

    void OnClientLeave(ClientSocketPtr& pClient)
    {
        if (_pNetEvent)
            _pNetEvent->OnNetLeave(pClient);
        _clients_change = true;
    }

    void WriteData(fd_set& fdWrite)
    {
        #ifdef _WIN32
        for (int n = 0; n < fdWrite.fd_count; n++)
        {
            auto iter  = _clients.find(fdWrite.fd_array[n]);
            if (iter != _clients.end())
            {
                if (0 == iter->second->sendDataImme())
                {
                    //closesocket(iter->first);
                    OnClientLeave(iter->second);
                    delete iter->second;
                    _clients.erase(iter->first);
                }
            }
        }
        #else
        
        std::vector<ClientSocketPtr> temp;
        for (auto& iter : _clients)
        {
            if (FD_ISSET(iter.first, &fdWrite))
            {
                if (-1 == iter.second->sendDataImme())
                {
                    OnClientLeave(iter.second);
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
                    OnClientLeave(iter->second);
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
                    OnClientLeave(iter.second);
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

        std::cout << "CellServer:" << _id << ".Close() 1" << std::endl;
        _taskServer.Close();
        _thread.Close();
        std::cout << "CellServer:" << _id << ".Close() 2" << std::endl;
    }    
    
    int recvData(ClientSocketPtr pClient)
    {
        int nLen = pClient->recvData();
        if(nLen <= 0)
        {
            return -1;
        }
        _pNetEvent->OnNetRecv(pClient);
        while(pClient->hasMsg())
        {
            onNetMsg(pClient,pClient->frontMsg());
            pClient->popFrontMsg();
        }
        //pClient->resetDTHeart();
        //Move msgBuf pointer to it's tail
        return 1;
    }

    //response net msg
    virtual void onNetMsg(ClientSocketPtr pClient,netmsg_DataHeader* header)
    {
        _pNetEvent->OnNetMsg(this,pClient,header);
    }
};
#endif  //!__CELLSERVER__H__