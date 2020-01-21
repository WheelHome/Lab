#ifndef CELLTIMESTAMP_HPP_
#define CELLTIMESTAMP_HPP_

#include <chrono>
using namespace std::chrono;
class CELLTimestamp
{
protected:
    time_point<high_resolution_clock> _begin;
public:
    CELLTimestamp()
    {
        update();
    }
    ~CELLTimestamp()
    {

    }

    void update()
    {
        _begin = high_resolution_clock::now();
    }

    //get epalsed second
    double getEpalsedSecond()
    {
        return getElapsedTimeInMIcroSec() * 0.000001;
    }

    //get millisecond
    double getElapsedTimeInMilliSec()
    {
        return getElapsedTimeInMIcroSec() * 0.001;
    }

    //get microsecond
    long long  getElapsedTimeInMIcroSec()
    {
        return duration_cast<microseconds>(high_resolution_clock::now()  - _begin).count() ;
    }
};
#endif