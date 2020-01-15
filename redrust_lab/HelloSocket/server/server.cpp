#include <thread>
#include "messageHeader.hpp"
#include "EasyTcpServer.hpp"

class MyServer : public EasyTcpServer
{
public:

    virtual void OnNetLeave(ClientSocket* pClient)
    {
        _clientCount--;
        std::cout << "client leave " << pClient->getSockfd() << std::endl;
    }

    virtual void OnNetMsg(ClientSocket* pClient,DataHeader* header)
    {
        _msgCount++;
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            Login* login = (Login*)header;
            LoginResult ret;
            pClient->sendData(&ret);
            break;
        }
        case CMD_LOGOUT:
        {
            break;
        }
        case CMD_ERROR:
        {
            break;
        }
        default:
        {
        }
        }
    }

    virtual void OnNetJoin(ClientSocket* pClient)
    {
        _clientCount++;
        std::cout << "client join " << pClient->getSockfd() << std::endl;
    }


    virtual void OnNetRecv(ClientSocket* pClient)
    {
        _recvCount++;
    }
private:

};

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
    MyServer server;
    server.initSocket();
    server.Bind(nullptr,4567);
    server.Listen(1000);
    server.Start(4);
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread

    while(g_bExit)
    {
        server.onRun();
    }
    return 0;
}
