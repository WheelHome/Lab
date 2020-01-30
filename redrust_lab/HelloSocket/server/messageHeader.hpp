#ifndef _MESSAGEHEADER_HPP_
#define _MESSAGEHEADER_HPP_

#include <cstring>
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
    DataHeader()
    {
        dataLength = sizeof(DataHeader);
        cmd = CMD_ERROR;
    }
    short dataLength;
    short cmd;  //command

};

//DataPackage
struct netmsg_Login: public DataHeader
{
    netmsg_Login()
    {
        dataLength = sizeof(netmsg_Login);
        cmd = CMD_LOGIN;
    }
    netmsg_Login(const char* loginName,const char* loginPwd)
    {
        strcpy(userName,loginName);
        strcpy(passWord,loginPwd);
        dataLength = sizeof(netmsg_Login);
        cmd = CMD_LOGIN;
    }
    char userName[32];
    char passWord[32];
    char data[906];
};

struct netmsg_LoginR: public DataHeader
{
    netmsg_LoginR()
    {
        dataLength = sizeof(netmsg_LoginR);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
    char data[92];
};

struct netmsg_Logout: public DataHeader
{
    netmsg_Logout()
    {
        dataLength = sizeof(netmsg_Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct netmsg_LogoutR: public DataHeader
{
    netmsg_LogoutR()
    {
        dataLength = sizeof(netmsg_LogoutR);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

struct netmsg_NewUserJoin : public DataHeader
{
    netmsg_NewUserJoin()
    {
        dataLength = sizeof(netmsg_NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;

    /* data */
};
#endif