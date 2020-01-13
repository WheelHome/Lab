
#include "messageHeader.hpp"
#include "EasyTcpClient.hpp"


#define cCount 1000
#define tCount 8
bool g_bExit = true;
void cmdThread()
{
    while(g_bExit)
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
void sendThread(int id)//four thread 1~4
{
    int c = (cCount) / tCount;
    int begin = (id-1)*c;
    int end  = id*c;
    EasyTcpClient* client[cCount];
    char ip[] = "127.0.0.1";
    for(int i = begin; i < end; i++)
    {
        client[i] = new EasyTcpClient();
    }
    for(int i = begin; i < end; i++)
    {
        if(client[i]->Connect(ip,4567) < 0)
        {
            std::cout <<"Client="<< i << "Connect error" << std::endl;
        }
    }
    Login login = {"jack","pass"};
    while(g_bExit)
    {
        for(int i = begin; i < end; i++)
        {
            if(client[i]->sendData(&login) == -1)
            {
                std::cout << "Send error" << std::endl;
                std::cout << errno << std::endl;
            }
            client[i]->onRun();
        }
    }
    for(int i = begin; i < end; i++)
    {
        client[i]->CLose();
        delete client[i];
    }
}

int main()
{
    //launch ui thread
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread

    for(int n = 0;n < tCount;n++)
    {
        //launch send thread
        std::thread t1(sendThread,n + 1);
        t1.detach();    //detach from main thread
    }

    while(g_bExit)
    {
        sleep(100);
    }
    return 0;
}
