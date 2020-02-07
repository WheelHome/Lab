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
    CMD_C2S_HEART,
    CMD_S2C_HEART,
    CMD_ERROR
};
struct netmsg_DataHeader
{
    /* data */
    netmsg_DataHeader()
    {
        dataLength = sizeof(netmsg_DataHeader);
        cmd = CMD_ERROR;
    }
    unsigned short dataLength;
    unsigned short cmd;  //command
};

//DataPackage
struct netmsg_Login: public netmsg_DataHeader
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

struct netmsg_LoginR: public netmsg_DataHeader
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

struct netmsg_Logout: public netmsg_DataHeader
{
    netmsg_Logout()
    {
        dataLength = sizeof(netmsg_Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct netmsg_LogoutR: public netmsg_DataHeader
{
    netmsg_LogoutR()
    {
        dataLength = sizeof(netmsg_LogoutR);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

struct netmsg_NewUserJoin : public netmsg_DataHeader
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

struct netmsg_C2S_Heart : public netmsg_DataHeader
{
    netmsg_C2S_Heart()
    {
        dataLength = sizeof(netmsg_C2S_Heart);
        cmd = CMD_C2S_HEART;
    }
    /* data */
};

struct netmsg_S2C_Heart : public netmsg_DataHeader
{
    netmsg_S2C_Heart()
    {
        dataLength = sizeof(netmsg_S2C_Heart);
        cmd = CMD_S2C_HEART;
    }
    /* data */
};
#endif