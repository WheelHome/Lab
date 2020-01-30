
#include "messageHeader.hpp"
#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"

#include <atomic>

#define cCount 200
#define tCount 4
bool g_bExit = true;
std::atomic_int sendCount(0);
std::atomic_int readyCount(0);

void recvThread(EasyTcpClient* client[],int begin,int end)
{
    while(g_bExit)
    {
        for(int i = begin; i < end; i++)
        {
            client[i]->onRun();
        }
    }
}

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

    std::cout << "thread=" << id << " Connect<begin=" << begin << ",end=" << end << std::endl;

    readyCount++;
    while(readyCount < tCount)
    {
        //wait for other thread to ready for sending data
        std::chrono::microseconds t(10);
        std::this_thread::sleep_for(t);
    }

    std::thread t1(recvThread,client,begin,end);
    t1.detach();

    netmsg_Login netmsg_Login = {"jack","pass"};
    const int nLen = sizeof(netmsg_Login);
    CELLTimestamp Time;
    while(g_bExit)
    {
        for(int i = begin; i < end; i++)
        {
            if(Time.getEpalsedSecond() >= 1.0)
            {
                if(client[i]->sendData(&netmsg_Login) == -1)
                {
                    std::cout << "Send error" << std::endl;
                    std::cout << errno << std::endl;
                }
                else
                {
                    sendCount++;
                }
                Time.update();
            }
            std::chrono::microseconds t(10);
            std::this_thread::sleep_for(t);
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

    for(int n = 0; n < tCount; n++)
    {
        //launch send thread
        std::thread t1(sendThread,n + 1);
        t1.detach();    //detach from main thread
    }

    CELLTimestamp tTime;
    while(g_bExit)
    {
        auto t = tTime.getEpalsedSecond();
        if(t >= 1.0)
        {
            std::cout << "thread=" << tCount << " clients=" << cCount << " time=" << t << " send=" << (int)sendCount / t  << std::endl;
            tTime.update();
            sendCount = 0;
        }
        sleep(1);
    }
    return 0;
}