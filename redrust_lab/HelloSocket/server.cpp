#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
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
        //DataHeaderBuffer
        char szRecv[1024] = {};
        int nLen = recv(_cSock,szRecv,sizeof(DataHeader),0);
        DataHeader* header = (DataHeader*)szRecv;
        //std::cout << "Received command  " << _recvBuf << std::cout;
        if(nLen <= 0)
        {
            std::cout << "Client quited." << std::endl;
            break;
        }
        //std::cout << "Received command " << header.cmd << " dataLength:" << header.dataLength << std::endl;
       /* if(nLen >= sizeof(DataHeader))
        {

        }*/
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
    closesocket(_sock);

    WSACleanup();
    return 0;
}