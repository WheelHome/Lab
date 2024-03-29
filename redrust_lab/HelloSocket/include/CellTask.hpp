#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include <list>
#include "CellThread.hpp"

/*class CellTask
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
    ClientSocketPtr _pClient;
    netmsg_DataHeader* _pHeader;
    std::mutex _mutex;
public:
    CellSendMsgToClientTask(ClientSocketPtr pClient,netmsg_DataHeader* header)
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

typedef std::shared_ptr<CellTask> CellTaskPtr;

typedef  std::shared_ptr<CellSendMsgToClientTask> CellSendMsgToClientTaskPtr;
*/

class CellTaskServer
{
    typedef std::function<void()> CellTask;
private:
    std::list<CellTask> _tasks;
    std::list<CellTask> _tasksBuf;
    CellThread _thread;
    std::mutex _mutex;
public:
    int _serverId = -1;
public:
    CellTaskServer()
    {

    }

    ~CellTaskServer()
    {
    }

    void addTask(CellTask task)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }

    void Start()
    {
        _thread.Start(nullptr,[this](CellThread& pThread){
            onRun(pThread);
        });
    }

    void onRun(CellThread& pThread)
    {
        while(pThread.isRun())
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
                pTask();
            }
            _tasks.clear();
        }
        for(auto pTask : _tasksBuf)
        {
            pTask();
        }
        std::cout << "CellTaskServer: "<< _serverId <<"onRun" << std::endl;
    }

    void Close()
    {
        std::cout << "CellTaskServer: "<< _serverId <<".Close() 1" << std::endl;
        _thread.Close();
        std::cout << "CellTaskServer: "<< _serverId <<".Close() 2" << std::endl;
    }
};

#endif
