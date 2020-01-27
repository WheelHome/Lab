#include "Allocator.h"

#include <thread>
#include "messageHeader.hpp"
#include "EasyTcpServer.hpp"

class MyServer : public EasyTcpServer
{
public:

    virtual void OnNetLeave(ClientSocketPtr& pClient)
    {
        EasyTcpServer::OnNetLeave(pClient);
    }

    virtual void OnNetMsg(CellServer* pCellServer,ClientSocketPtr& pClient,DataHeader* header)
    {
        EasyTcpServer::OnNetMsg(pCellServer,pClient,header);
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            pCellServer->addSendTask(pClient,header);
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

    virtual void OnNetJoin(ClientSocketPtr& pClient)
    {
        EasyTcpServer::OnNetJoin(pClient);
    }


    virtual void OnNetRecv(ClientSocketPtr& pClient)
    {
        EasyTcpServer::OnNetRecv(pClient);
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
    server.Listen(1023);
    server.Start(4);
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread


    while(g_bExit)
    {
        server.onRun();
    }
    return 0;
}
