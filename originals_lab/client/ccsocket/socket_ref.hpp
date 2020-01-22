#ifndef SOCKET_REF_HPP
#define SOCKET_REF_HPP

#include "util/ref.hpp"
#include "ccsocket.h"

struct SocketRef : Ref
{
    SocketRef()
    {
        m_socket.InitSocket();
    }

    void Delete(void* obj) override
    {
        delete (SocketRef*) obj;
    }

    ~SocketRef() = default;

    cc::ccSocket m_socket;
};

#endif

