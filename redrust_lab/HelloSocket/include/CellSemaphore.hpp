#ifndef __CELLSEMAPHORE__H__
#define __CELLSEMAPHORE__H__

#include <condition_variable>
#include "Cell.hpp"

class CellSemaphore
{
private:
    //waitting count
    int _wait = 0;
    //wakeup count
    int _wakeup = 0;
    //block wait-condition variable
    std::condition_variable _cv;
    std::mutex _mutex;
public:
    //wakeup current thread
    void wakeup()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if(++_wait <= 0)
        {
            ++_wakeup;
            _cv.notify_one();
        }else
        {
            std::cout << "CellSemaphore wakeup error." << std::endl;
        }
    }

    //block current thread
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if(--_wait < 0)
        {
            _cv.wait(lock,[this]()->bool{
             return    _wakeup > 0;
            });
            --_wakeup;
        }
    }

    CellSemaphore(/* args */)
    {

    }

    ~CellSemaphore()
    {

    }
};


#endif  //!__CELLSEMAPHORE__H__