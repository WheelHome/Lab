
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
        else if(strcmp(cmdBuf,"login") == 0)
        {
            Login login;
            strcpy(login.userName,"Jack");
            strcpy(login.passWord,"pass");
            client->sendData(&login);
        }
        else if(strcmp(cmdBuf,"logout") == 0)
        {
            Logout logout;
            strcpy(logout.userName,"Jack");
            client->sendData(&logout);
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
    client.Connect("127.0.0.1",4567);
    //launch thread function
    std::thread t1(cmdThread,&client);
    t1.detach();    //detach from main thread

    while(g_bExit)
    {
        client.onRun();
    }
    client.CLose();
    return 0;
}
