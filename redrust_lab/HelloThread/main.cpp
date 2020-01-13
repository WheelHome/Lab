#include <iostream>
#include <thread>
#include <mutex>    //lock
#include <atomic>   //atomic operating
#include "CELLTimestamp.hpp"

//Atomic operating

const int tCount = 4;
std::atomic_int sum(0);
std::mutex m;
void workFun(int index)
{
    for(int n = 0 ; n < 20000 ; n++)
    {
        std::lock_guard<std::mutex> lg(m);//self unlocking
        m.lock();   //critical section begin
        sum++;
        std::cout << index << " Hello,other thread." << n << std::endl;
        m.unlock(); //critical section end
    }
}
int main()
{
    std::thread* t[tCount];
    for(int n = 0 ; n < tCount ; n++)
    {
        t[n] = new std::thread(workFun,n);
    }
    CELLTimestamp tTime;
    for(int n = 0 ; n < tCount ; n++)
    {
        t[n]->join();
        // t[n]->detach();
    }
    std::cout << tTime.getElapsedTimeInMilliSec() << std::endl;
    t.detach();//Main thread is isolating to subthread    //preemptive
    t.join();//Main thread wait to subthread had finising.
    std::cout << "sum=" << sum << std::endl;
    std::cout << "Hello,Main thread." << std::endl;
    for(int n = 0 ; n < tCount ; n++)
    {
        delete t[n];
    }
    return 0;
}