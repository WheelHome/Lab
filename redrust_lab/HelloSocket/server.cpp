#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")
int main()
{
    WORD ver = MAKEWORD(2,2);
    WSADATA dat;
    WSAStartup(ver,&dat);

    SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = hton(4567);
    _sin.sin_addr.s_un.S_addr = INADDR_ANY;

    if(bind(_sock,(sockaddr_in*)&_sin,sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        std::cout << "Bind Socket Error!" << std::endl;
        return -1;
    }

    if(listen(_sock,5) == SOCKET_ERROR)
    {
        std::cout << "Listen Socket Error!" << std::endl;
        return -2;
    }

    sockaddr_in clientAddr = {};

    int nAddrLen = sizeof(clientAddr);
    SOCKET _cSock = INVALID_SOCKET;

    _cSock = accept(_sock,(sockaddr_in*)&clientAddr,&nAddrLen);

    if(_cSock == INVALID_SOCKET)
    {
        std::cout << "Received wrong client socket！"  << std::endl;
        return -3;
    }
    std::cout << "new client joined in and it's IP=" << inet_ntoa(clientAddr.sin_addr)<< std::endl;

    //char cmdBuf[128]  = {};
    while(true)
    {
        int nLen = recv(_cSock,_recvBuf,128,0);
        std::cout << "Recvived command  " << _recvBuf << std::cout;
        if(nLen <= 0)
        {
            std::cout << "Client quited." << std::endl;
            break;
        }
        if( strcmp(_recvBuf,"getName") == 0)
        {
            char msgBuf[] = "I'm Server!";
            send(_cSock,msgBuf,strlen(msgBuf)+1,0);
        }
        else if( strcmp(_recvBuf,"getAge") == 0 )
        {
            char msgBuf[] = "I'm 80!";
            send(_cSock,msgBuf,strlen(msgBuf)+1,0);
        } 
        else
        {
            char msgBuf[] = "???";
            send(_cSock,msgBuf,strlen(msgBuf)+1,0);
        }
    }
    closesocket(_sock);

    WSACleanup();
    return 0;
}