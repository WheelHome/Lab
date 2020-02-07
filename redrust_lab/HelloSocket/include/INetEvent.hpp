#ifndef __INETEVENT__H__
#define __INETEVENT__H__

#include "Cell.hpp"
class CellServer;
class INetEvent 
{
private:

public:
    //Client quited event
    virtual void OnNetLeave(ClientSocketPtr& pClient) = 0;
    //Client msg event
    virtual void OnNetMsg(CellServer* pCellServer,ClientSocketPtr& pClient,netmsg_DataHeader* header) = 0;
    //recv event
    virtual void OnNetRecv(ClientSocketPtr& pClient) = 0;
    //new client join event
    virtual void OnNetJoin(ClientSocketPtr& pClient) = 0;
};
#endif  //!__INETEVENT__H__