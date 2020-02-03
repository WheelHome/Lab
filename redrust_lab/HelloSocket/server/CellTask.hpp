#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include <list>
#include <functional>


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
    CellSemaphore _sem;
    std::mutex _mutex;
    std::thread  _thread;
    bool _isRun = false;
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
        _isRun = true;
        _thread = std::thread(std::mem_fun(&CellTaskServer::onRun),this);
        _thread.detach();
    }

    void onRun()
    {
        while(_isRun)
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
        std::cout << "CellTaskServer: "<< _serverId <<"onRun" << std::endl;
        _sem.wakeup();
    }

    void Close()
    {
        std::cout << "CellTaskServer: "<< _serverId <<".Close() 1" << std::endl;
        _isRun = false;
        _sem.wait();
        std::cout << "CellTaskServer: "<< _serverId <<".Close() 2" << std::endl;
    }
};

#endif
