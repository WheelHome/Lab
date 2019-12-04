#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <WinSock2.h>

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
        if( strcmp(cmdBuf,"exit") == 0)
        {
            break;
        }
        else if(strcmp(cmdBuf,"login") == 0)
        {
            Login login;
            strcpy(login.userName,"Jack");
            strcpy(login.passWord,"pass");
            DataHeader dh = {sizeof(login),CMD_LOGIN};

            //Send to command and header
            //send(_sock,(const char*)&dh,sizeof(dh), 0 );
            send(_sock,(const char*)&login,strlen(login), 0 );

            //Received from server
            LoginResult loginRet = {};
            //recv(_sock,(char*)&retHeader,sizeof(retHeader),0);
            recv(_sock,(char*)&loginRet,sizeof(loginRet),0);
            std::cout <<"LoginResult:" << LoginResult << std::endl;
        }
        else if(strcmp(cmdBuf,"logout") == 0)
        {
            Logout logout = {};
            strcpy(logout.userName,"Jack");

            send(_sock,(const char*)&logout,strlen(logout), 0 );

            LogoutResult logoutRet = {};
            recv(_sock,(char*)&logoutRet,sizeof(logoutRet),0);
            std::cout <<"LogoutResult:" << LogoutResult << std::endl;
        }
        else
        {
            std::cout << "The command is not support!" << std::endl;
        }
        char recvBuf[256] = {};
        int nlen = recv(_sock,recvBuf,256,0);
        if(nlen > 0)
        {
            DataPackage* dp = (DataPackage*)recvBuf;
            std::cout << "Received data age:" << dp->age <<" name:" << dp->name << std::endl;
            //std::cout << " The data received from server  which is " << recvBuf << std::endl;
        }

    }
    closesocket(_sock);
    WSACleanup();
    return 0;
}