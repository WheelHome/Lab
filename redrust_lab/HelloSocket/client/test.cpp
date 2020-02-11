#include "EasyTcpClient.hpp"
#include "CellMsgStream.hpp"

class MyClient : public EasyTcpClient
{
private:

public:
    virtual void onNetMsg(netmsg_DataHeader* header)
    {
        switch (header->cmd)
        {
        case CMD_LOGOUT_RESULT:
        {
            int8_t n1;
            int16_t n2;
            int32_t n3;
            float n4;
            double n5;
            CellRecvStream r(header);
            r.getNetCMD();
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

             /*   len = cellStream.readArray(b,len);
            for(int i=0;i<len;i++)
            {
                std::cout << b[i] << std::endl;
            }*/

            break;
        }
        case CMD_ERROR:
        {

        }
        default:
        {
        }
        }
    }
    MyClient(/* args */)
    {

    }

    ~MyClient()
    {

    }
};

int main()
{
    CellSendStream cellStream;
    cellStream.setNetCMD(CMD_LOGOUT);
    cellStream.writeInt8(67);
    cellStream.writeInt16(2);
    cellStream.writeInt32(3);
    cellStream.writeFloat(4.5f);
    cellStream.writeDouble(6.7);

    int a[] = {1,2,3,4,5};
    cellStream.writeArray(a,5);
    cellStream.finish();
 


    MyClient client;
    client.Connect("127.0.0.1",4567);
    client.sendData(cellStream.data(),cellStream.length());
    while(client.isRun())
    {
        client.onRun();
        std::chrono::microseconds t(10);
        std::this_thread::sleep_for(t);
    }
    return 0;
}