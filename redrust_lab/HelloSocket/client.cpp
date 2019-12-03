#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")
int main()
{
    WORD ver = MAKEWORD(2,2);
    WSADATA dat;
    WSAStartup(ver,&dat);

    SOCKET _sock = socket(AD_INET,SOCK_STREAM,0);
    if(_sock == INVALID_SOCKET)
    {
        std::cout << "Create socket  error!" << std::endl;
        return -1;
    }
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock,(sockaddr_in*)&_sin,sizeof(sockaddr_in));

    if( ret == SOCKET_ERROR)
    {
        std::cout << "Connect socket error!" << std::endl;
        return -2;
    }

    while(true)
    {
        char cmdBuf[128] = {};
        std::cin >> cmdBuf;
        if( stecmp(cmdBuf,"exit"))
        {
            break;
        }
        else
        {
            send(_sock,cmdBuf,strlen(cmdBuf) + 1, 0 );

        }
        char recvBuf[256] = {};
        int nlen = recv(_sock,recvBuf,256,0);
        if(nlen > 0)
        {
            std::cout << " The data received from server  which is " << recvBuf << std::endl;
        }
        
    }
    closesocket(_sock);
    WSACleanup();
    return 0;
}