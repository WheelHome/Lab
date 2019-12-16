
#include "messageHeader.hpp"
#include "EasyTcpClient.hpp"


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
    const int cCount = 1000;
    EasyTcpClient* client[cCount];
    char ip[] = "127.0.0.1";
    for(int i = 0; i < cCount; i++)
    {
        if(!g_bExit)
        {
            return 0;
        }
        client[i] = new EasyTcpClient();
    }
    for(int i = 0; i < cCount;i++)
    {
        if(!g_bExit)
        {
            return 0;
        }
        client[i]->Connect(ip,4567);
    }
    //launch thread function
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread

    Login login = {"jack","pass"};
    while(g_bExit)
    {
        for(int i = 0; i < cCount; i++)
        {
            client[i]->sendData(&login);
            //client[i]->onRun();
        }
        //client.onRun();
       // client.sendData(&login);
    }
    for(int i = 0;i < cCount;i++)
    {
        client[i]->CLose();
        delete client[i];
    }
    return 0;
}
