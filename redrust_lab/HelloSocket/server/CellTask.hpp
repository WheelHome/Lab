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

typedef std::shared_ptr<CellTask> CellTaskPtr;

class CellSendMsgToClientTask:public CellTask
{
private:
    ClientSocketPtr _pClient;
    DataHeader* _pHeader;
    std::mutex _mutex;
public:
    CellSendMsgToClientTask(ClientSocketPtr pClient,DataHeader* header)
    {
        _pClient = pClient;
        _pHeader = header;
    }

    CellSendMsgToClientTask()
    {

    }

    ~CellSendMsgToClientTask()
    {
        _pHeader = nullptr;
    }

    virtual void doTask()
    {
        _mutex.lock();
        _pClient->sendData(_pHeader);
        _mutex.unlock();
    }

};

typedef  std::shared_ptr<CellSendMsgToClientTask> CellSendMsgToClientTaskPtr;

class CellTaskServer
{
private:
    std::list<CellTaskPtr> _tasks;
    std::list<CellTaskPtr> _tasksBuf;
    std::mutex _mutex;
    std::thread  _thread;
public:
    CellTaskServer()
    {

    }

    ~CellTaskServer()
    {
    }

    void addTask(CellTaskPtr &task)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }

    void Start()
    {
        _thread = std::thread(std::mem_fun(&CellTaskServer::onRun),this);
        _thread.detach();
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
            }
            _tasks.clear();
        }
    }


};

#endif
