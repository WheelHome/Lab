
#include "messageHeader.hpp"
#include "EasyTcpClient.hpp"


bool g_bExit = true;
void cmdThread(EasyTcpClient* client)
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
    EasyTcpClient client;
    client.initSocket();
    char ip[] = "127.0.0.1";
    client.Connect(ip,4567);
    //launch thread function
    std::thread t1(cmdThread,&client);
    t1.detach();    //detach from main thread

    Login login = {"jack","pass"};
    while(g_bExit)
    {
        client.onRun();
        client.sendData(&login);
    }
    client.CLose();
    return 0;
}
