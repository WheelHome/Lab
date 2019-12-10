
#include "messageHeader.hpp"
#include "EasyTcpServer.hpp"

int main()
{
    EasyTcpServer server;
    server.initSocket();
    server.Bind(nullptr,4567);
    server.Listen(5);

    while(server.isRun())
    {
        server.onRun();
    }
    return 0;
}
