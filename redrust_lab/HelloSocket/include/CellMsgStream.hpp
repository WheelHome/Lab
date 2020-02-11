#ifndef __CELLMSGSTREAM__H__
#define __CELLMSGSTREAM__H__

#include "CellStream.hpp"

class CellRecvStream : public CellStream
{
private:
    /* data */
public:
    CellRecvStream(netmsg_DataHeader* header)
        :CellStream((char*)header,header->dataLength)
    {
        int16_t temp;
        readInt16(temp);
        getNetCMD();
    }   

    uint16_t getNetCMD()
    {
        uint16_t cmd = CMD_ERROR;
        read<uint16_t>(cmd);
        return cmd;
    }
};

class CellSendStream : public CellStream
{
private:
    /* data */
public:
    CellSendStream(char* pData,int nSize,bool bDelete = false)
        :CellStream(pData,nSize,bDelete)
    {
        write<uint16_t>(0);
    }

    CellSendStream(int nSize = 1024)
        :CellStream(nSize)
    {
        write<uint16_t>(0);
    }

    void finish()
    {
        int pos = length();
        setWritePos(0);
        write<uint16_t>(pos);
        setWritePos(pos);
    }

    void setNetCMD(uint16_t cmd)
    {
        write<uint16_t>(cmd);
    }
};
#endif  //!__CELLMSGSTREAM__H__