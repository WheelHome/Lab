#include "Allocator.h"
#include "CELLTimestamp.hpp"
#include <thread>

const int tCount = 4;
void workFun(int index)
{

    char* data[1100];
    for(size_t i = 0 ;i < 1100; i++)
    {
        data[i] = new char[1 + i];
    }

    for(size_t i = 0 ;i < 1100; i++)
    {
        delete[] data[i];
    }
}

int main()
{
    /*
    //1
    char* data1 = new char [128];
    delete[] data1;

    //2
    char* data2 = new char;
    delete data2;

    //3
    char* data3 = new char [64];
    delete[] data3;*/

    //4}
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
    std::cout << "Hello,Main thread." << std::endl;
    for(int n = 0 ; n < tCount ; n++)
    {
        delete t[n];
    }
    return 0;
}