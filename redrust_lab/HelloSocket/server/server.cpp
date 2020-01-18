#include <thread>
#include "messageHeader.hpp"
#include "EasyTcpServer.hpp"

class MyServer : public EasyTcpServer
{
public:

    virtual void OnNetLeave(ClientSocket* pClient)
    {
        EasyTcpServer::OnNetLeave(pClient);
    }

    virtual void OnNetMsg(CellServer* pCellServer,ClientSocket* pClient,DataHeader* header)
    {
        EasyTcpServer::OnNetMsg(pCellServer,pClient,header);
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            LoginResult* ret = new LoginResult();
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

    virtual void OnNetJoin(ClientSocket* pClient)
    {
        EasyTcpServer::OnNetJoin(pClient);
    }


    virtual void OnNetRecv(ClientSocket* pClient)
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
    server.Listen(1000);
    server.Start(1);
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread


    while(g_bExit)
    {
        server.onRun();
    }
    return 0;
}
