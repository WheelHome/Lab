#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include <list>

class CellTask
{
private:

public:
    CellTask()
    {

    }

    virtual ~CellTask()
    {

    }

    virtual void doTask()
    {

    }

};

class CellSendMsgToClientTask:public CellTask
{
private:
    ClientSocket* _pClient;
    DataHeader* _pHeader;
    std::mutex _mutex;
public:
    CellSendMsgToClientTask(ClientSocket* pClient,DataHeader* header)
    {
        _pClient = pClient;
        _pHeader = header;
    }

    CellSendMsgToClientTask()
    {

    }

    ~CellSendMsgToClientTask()
    {
        _pClient = nullptr;
        _pHeader = nullptr;
    }

    virtual void doTask()
    {
        _mutex.lock();
        _pClient->sendData(_pHeader);
        _mutex.unlock();
    }

};

class CellTaskServer
{
private:
    std::list<CellTask*> _tasks;
    std::list<CellTask*> _tasksBuf;
    std::mutex _mutex;
    std::thread* _thread;
public:
    CellTaskServer()
    {

    }

    ~CellTaskServer()
    {
        if(_thread->joinable())
        {
            _thread->join();
            delete _thread;
        }
    }

    void addTask(ClientSocket* pClient,DataHeader* header)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        CellSendMsgToClientTask* task = new CellSendMsgToClientTask(pClient,header);
        _tasksBuf.push_back(task);
    }

    void Start()
    {
        _thread = new std::thread(std::mem_fun(&CellTaskServer::onRun),this);
        _thread->detach();
    }

    void onRun()
    {
        while(true)
        {
            _mutex.lock();
            if(!_tasksBuf.empty())
            {
                for(auto pTask : _tasksBuf)
                {
                    _tasks.push_back(pTask);
                }
                _tasksBuf.clear();
            }
            _mutex.unlock();
            if(_tasksBuf.empty())
            {
                std::chrono::microseconds t(1);
                std::this_thread::sleep_for(t);
                continue;
            }
            for(auto pTask : _tasks)
            {
                pTask->doTask();
                delete pTask;
            }
            _tasks.clear();
        }
    }


};

#endif