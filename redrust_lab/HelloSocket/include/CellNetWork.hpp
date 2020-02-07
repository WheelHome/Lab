#ifndef __CELLNETWORK__H__
#define __CELLNETWORK__H__

#include "Cell.hpp"

class CellNetWork
{
private:
    /* data */
    CellNetWork(/* args */)
    {
        #ifdef _WIN32
        WORD ver = MAKEWORD(2,2);
        WSADATA dat;
        WSAStartup(ver,&dat);
        #endif
    }

    ~CellNetWork()
    {
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
public:

    static void Instance()
    {
        static CellNetWork cellNetWork;
    }
};


#endif  //!__CELLNETWORK__H__