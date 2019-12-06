
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
int processor(SOCKET _cSock)
{
    //DataHeaderBuffer
    char szRecv[1024] = {};
    int nLen = recv(_cSock,szRecv,sizeof(DataHeader),0);
    DataHeader* header = (DataHeader*)szRecv;
    //std::cout << "Received command  " << _recvBuf << std::cout;
    if(nLen <= 0)
    {
        std::cout << "Connection broken" << std::endl;
        return -1;
    }
    switch (header->cmd)
    {
    case CMD_LOGIN_RESULT:
    {
        recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        LoginResult* login = (LoginResult*)szRecv;
        std::cout << "Received command " << login->cmd << " dataLength:" << header->dataLength << std::endl;
        break;
    }
    case CMD_LOGOUT_RESULT:
    {
        recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        LogoutResult* logout = (LogoutResult*)szRecv;
        std::cout << "Received command " << logout->cmd << " dataLength:" << header->dataLength << std::endl;
        break;
    }
    case CMD_NEW_USER_JOIN:
    {
        recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader),0);
        NewUserJoin* newUserJoin = (NewUserJoin*)szRecv;
        std::cout << "Received command " << newUserJoin->cmd << " dataLength:" << header->dataLength  << std::endl;
        break;
    }
    default:
    {
    }
    }
}

bool g_bExit = true;
void cmdThread(SOCKET _sock)
{

        //thread

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
            else if(strcmp(cmdBuf,"login") == 0)
            {
                Login login;
                strcpy(login.userName,"Jack");
                strcpy(login.passWord,"pass");

                send(_sock,(const char*)&login,sizeof(Login),0);
            }
            else if(strcmp(cmdBuf,"logout") == 0)
            {
                Logout logout;
                strcpy(logout.userName,"Jack");
                send(_sock,(const char*)&logout,sizeof(Logout),0);
            }
            else
            {
                std::cout << "Not supported command!" << std::endl;
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

    SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
    if(_sock == INVALID_SOCKET)
    {
        std::cout << "Create socket  error!" << std::endl;
        return -1;
    }
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);

#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
    _sin.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

    int ret = connect(_sock,(const sockaddr*)&_sin,sizeof(sockaddr_in));

    if( ret == SOCKET_ERROR)
    {
        std::cout << "Connect socket error!" << std::endl;
        return -2;
    }

    //launch thread function
    std::thread t1(cmdThread,_sock);
    t1.detach();    //detach from main thread

    while(g_bExit)
    {
        fd_set fdReads;
        FD_ZERO(&fdReads);
        FD_SET(_sock,&fdReads);
        timeval t = {1,0};
        int ret = select(_sock,nullptr,nullptr,nullptr,&t);
        if(ret < 0)
        {
            std::cout << "select completed!" << std::endl;
            break;
        }
        if(FD_ISSET(_sock,&fdReads))
        {
            FD_CLR(_sock,&fdReads);
            if(processor(_sock) == -1)
            {
                std::cout << "Select finished" << std::endl;
                break;
            }
        }

    }
    closesocket(_sock);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}