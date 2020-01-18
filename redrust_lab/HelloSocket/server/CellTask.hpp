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
        _pClient->sendData(_pHeader);
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
        if(_thread)
            delete _thread;
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
    }

    void onRun()
    {
        while(true)
        {
            if(!_tasksBuf.empty())
            {
                std::lock_guard<std::mutex> lock(_mutex);
                for(auto pTask : _tasksBuf)
                {
                    _tasks.push_back(pTask);
                }
                _tasksBuf.clear();
            }
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
