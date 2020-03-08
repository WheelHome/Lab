#include "EasyTcpServer.hpp"
#include "CellMsgStream.hpp"
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
            int8_t n1;
            int16_t n2;
            int32_t n3;
            float n4;
            double n5;
            CellRecvStream r(header);
            int16_t temp;

            int b[32] = {};
            uint32_t len = 32;
            r.readInt8(n1);
            std::cout << n1 << std::endl;
            r.readInt16(n2);
            std::cout << n2 << std::endl;
            r.readInt32(n3);
            std::cout << n3 << std::endl;
            r.readFloat(n4);
            std::cout << n4 << std::endl;
            r.readDouble(n5);
            std::cout << n5 << std::endl;
            std::cout << len << std::endl;

            CellSendStream cellStream;
            cellStream.setNetCMD(CMD_LOGIN_RESULT);
            cellStream.writeInt8(67);
            cellStream.writeInt16(2);
            cellStream.writeInt32(3);
            cellStream.writeFloat(4.5f);
            cellStream.writeDouble(6.7);

            int a[] = {1,2,3,4,5};
            cellStream.writeArray(a,5);
            cellStream.finish();
            pClient->sendData(cellStream.data(),cellStream.length());
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
    server.Start(1);
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
