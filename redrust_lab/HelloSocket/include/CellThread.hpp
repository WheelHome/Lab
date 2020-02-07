#ifndef __CELLTHREAD__H__
#define __CELLTHREAD__H__

#include "Cell.hpp"
#include "CellSemaphore.hpp"

class CellThread
{
private:
    typedef std::function<void(CellThread&)> EventCall;
private:
    EventCall _onCreate;
    EventCall _onRun;
    EventCall _onDestory;
    CellSemaphore _sem;
    std::mutex _mutex;
    bool _isRun = false;
public:
    void Start(EventCall onCreate  = nullptr,
        EventCall onRun = nullptr,
        EventCall onClose = nullptr)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(!_isRun)
        {
            _isRun = true;
            if(onCreate)
            {
                _onCreate = onCreate;
            }
            if(onRun)
            {
                _onRun = onRun;
            }
            if(onClose)
            {
                _onDestory = onClose;
            }
            std::thread t(std::mem_fun(&CellThread::OnWork),this);
            t.detach();
        }
    }

    void Close()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_isRun)
        {
            _isRun = false;
            _sem.wait();
        }
    }

    //Exit current thread
    void Exit()
    {

    }

    bool isRun()
    {
        return _isRun;
    }

    CellThread(/* args */)
    {

    }

    ~CellThread()
    {

   
    }
protected:
    void OnWork()
    {
        if(_onCreate)
        {
            _onCreate(*this);
        }
        if(_onRun)
        {
            _onRun(*this);
        }
        if(_onDestory)
        {
            _onDestory(*this);
        }
        _sem.wakeup();
    }
};

#endif  //!__CELLTHREAD__H__