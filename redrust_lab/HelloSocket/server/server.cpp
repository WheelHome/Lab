#include "EasyTcpServer.hpp"

class MyServer : public EasyTcpServer
{
public:

    virtual void OnNetLeave(ClientSocketPtr& pClient)
    {
        EasyTcpServer::OnNetLeave(pClient);
    }

    virtual void OnNetMsg(CellServer* pCellServer,ClientSocketPtr& pClient,netmsg_DataHeader* header)
    {
        EasyTcpServer::OnNetMsg(pCellServer,pClient,header);
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            pClient->resetDTHeart();
            netmsg_LoginR ret;
            if(pClient->sendData(&ret) == 0)
            {
                //SendBuf is fulling.
             //   CellLogger::Instance().Info("%d" ,ret);
            }
            //pCellServer->addSendTask(pClient,header);
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
        case CMD_C2S_HEART:
        {
            pClient->resetDTHeart();
            netmsg_S2C_Heart ret;
            pClient->sendData(&ret);
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


int main()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    CellLogger::Instance().setLogPath("serverLog.txt","w");
    MyServer server;
    server.initSocket();
    server.Bind(nullptr,4567);
    server.Listen(1023);
    server.Start(4);
    /*
    std::thread t1(cmdThread);
    t1.detach();    //detach from main thread
    */

    while(true)
    {
        char cmdBuf[256] = {};
        std::cin >> cmdBuf;
        if(strcmp(cmdBuf,"exit") == 0)
        {
            std::cout << "Exited" << std::endl;
            server.CLose();
            break;
        }
        else
        {
            std::cout << "Not supported command!" << std::endl;
        }
    }
    exit(0);
}
