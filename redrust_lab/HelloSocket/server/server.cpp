
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <windows.h>
    #include <WinSock2.h>
    #pragma comment(lib,"ws2_32.lib")
#else
    #include <unistd.h>
    #include <vector>
    #include <sys/types.h>          /* See NOTES */
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <string.h>
    #include <thread>
    #include <iostream>
    #define SOCKET int
    #define closesocket close
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR (-1)
#endif


enum CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};
struct DataHeader
{
    /* data */
    short dataLength;
    short cmd;  //command

};

//DataPackage
struct Login: public DataHeader
{
    Login()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    DataHeader header;
    char userName[32];
    char passWord[32];
};

struct LoginResult: public DataHeader
{
    LoginResult()
    {
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
};

struct Logout: public DataHeader
{
    Logout()
    {
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct LogoutResult: public DataHeader
{
    LogoutResult()
    {
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

struct NewUserJoin : public DataHeader
{
    NewUserJoin()
    {
        dataLength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;

    /* data */
};


std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
    //DataHeaderBuffer
    char szRecv[1024] = {};
    int nLen = recv(_cSock,szRecv,sizeof(DataHeader),0);
    DataHeader* header = (DataHeader*)szRecv;
    //std::cout << "Received command  " << _recvBuf << std::cout;
    if(nLen <= 0)
    {
        std::cout << "Client quited." << std::endl;
        return -1;
    }
    switch (header->cmd)
    {
    case CMD_LOGIN:
    {
        recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        Login* login = (Login*)szRecv;
        LoginResult ret;
        std::cout << "Received command " << login->cmd << " dataLength:" << login->dataLength << " userName:" <<
                    login->userName << " userPassword:" << login->passWord<< std::endl;
        send(_cSock,(const char*)&ret,sizeof(LoginResult),0);
        break;
    }
    case CMD_LOGOUT:
    {
        Logout* logout = (Logout*)szRecv;
        recv(_cSock,szRecv +  sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        std::cout << "Received command " << logout->cmd << " dataLength:" << logout->dataLength << " userName:" <<
                    logout->userName << std::endl;
        LogoutResult ret;
        //send(_cSock,(const char*)&header,sizeof(DataHeader),0);
        send(_cSock,(const char*)&ret,sizeof(LogoutResult),0);
        break;
    }
    default:
    {
        DataHeader header = {0,CMD_ERROR};
        send(_cSock,(const char*)&header,sizeof(DataHeader),0);
    }
    }
}

int main()
{
#ifdef _WIN32
    WORD ver = MAKEWORD(2,2);
    WSADATA dat;
    WSAStartup(ver,&dat);
#endif

    SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);

#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
    _sin.sin_addr.s_addr = INADDR_ANY;
#endif

    if(bind(_sock,(const sockaddr*)&_sin,sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        std::cout << "Bind Socket Error!" << std::endl;
        return -1;
    }

    if(listen(_sock,5) == SOCKET_ERROR)
    {
        std::cout << "Listen Socket Error!" << std::endl;
        return -2;
    }


    //char cmdBuf[128]  = {};
    while(true)
    {
        // socket
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);

        FD_SET(_sock,&fdRead);
        FD_SET(_sock,&fdWrite);
        FD_SET(_sock,&fdExp);

        for(int n = 0;n < g_clients.size(); ++n)
        {
            FD_SET(g_clients[n],&fdRead);
        }
        //nfds 

        timeval t = {0,0};
        int ret = select(_sock + 1,&fdRead,&fdWrite,&fdExp,&t);
        if(ret < 0)
        {
            std::cout << "select error!" << std::endl;
            break;
        }
        if(FD_ISSET(_sock,&fdRead))
        {
            FD_CLR(_sock,&fdRead);
            sockaddr_in clientAddr = {};

            int nAddrLen = sizeof(clientAddr);
            SOCKET _cSock = INVALID_SOCKET;

            _cSock = accept(_sock,(sockaddr*)&clientAddr,(socklen_t*)&nAddrLen);

            if(_cSock == INVALID_SOCKET)
            {
                std::cout << "Received wrong client socket！"  << std::endl;
                break;
            }
            else
            {
                for(int n = 0;n < g_clients.size(); ++n)
                {
                    NewUserJoin userJoin = {};
                    send(g_clients[n],(const char*)&userJoin,sizeof(NewUserJoin),0);
                }
                std::cout << "new client joined in and it's IP=" << inet_ntoa(clientAddr.sin_addr)<< std::endl;
                g_clients.push_back(_cSock);    
            }
        }
        for(int n = 0;n <= fdRead.fd_count; ++n)
        {
            if(processor(fdRead.fd_array[n]) == -1)
            {
                auto iter = std::find(g_clients.begin(),g_clients.end(),fdRead.fd_array[n]);
                if(iter != g_clients.end())
                {
                    g_clients.erase(iter);
                }
            }
        }

        //std::cout << "Received command " << header.cmd << " dataLength:" << header.dataLength << std::endl;
        /* if(nLen >= sizeof(DataHeader))
         {

         }*/
    }
    for(size_t n = 0;n < g_clients.size(); ++n)
    {
        closesocket(g_clients[n]);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}