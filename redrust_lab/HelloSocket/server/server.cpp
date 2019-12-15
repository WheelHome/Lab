#include <thread>
#include "messageHeader.hpp"
#include "EasyTcpServer.hpp"

bool g_bExit = true;
void cmdThread()
{
    while(true)
    {
        char cmdBuf[256] = {};
        std::cin >> cmdBuf;
        if(strcmp(cmdBuf,"exit") == 0)
        {
            std::cout << "Exited" << std::endl;
            g_bExit = false;
            break ;
        }
        else
        {
            std::cout << "Not supported command!" << std::endl;
        }
    }
}
int main()
{
    EasyTcpServer server;
    server.initSocket();
    server.Bind(nullptr,4567);
    server.Listen(5);
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread

    while(g_bExit)
    {
        server.onRun();
    }
    return 0;
}
