#include "ccsocket.h"

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>

#include "../util/debugutil.hpp"

namespace cc
{
ccSocket::ccSocket()
    : m_socket(-1),
      m_port(0),
      m_recv_count(0),
      m_send_count(0)
{
}

ccSocket::~ccSocket()
{
    TRACE("send count :", m_send_count);
    TRACE("recv count :", m_recv_count);
    close(m_socket);
}

//! Socket

void ccSocket::InitSocket(int domain, int type, int protocol)
{
    if (m_socket != -1)
    {
        close(m_socket);
    }
    m_socket = socket(domain, type, protocol);
    assert(m_socket != -1);
}

void ccSocket::SetSocket(int socket)
{
    assert(m_socket != -1 && "socket is unavailable");
    m_socket = socket;
}

int ccSocket::GetSocket()
{
    return m_socket;
}

//! IP Address

void ccSocket::SetIP(const std::string& ip)
{
    TRACE("ip address :", ip);
    m_ip = ip;
}

const std::string& ccSocket::GetIP() const
{
    return m_ip;
}

//! Port

void ccSocket::SetPort(const uint32_t& port)
{
    TRACE("port :", port);
    m_port = port;
}

uint32_t ccSocket::GetPort()
{
    return m_port;
}

//! Port

int ccSocket::SetSockOpt(const int& level, const int& optname, const void *optval, const socklen_t& optlen)
{
    return setsockopt(m_socket, level, optname, optval, optlen);
}

int ccSocket::GetSockOpt(const int& level, const int& optname, void *optval, socklen_t* optlen)
{
    return getsockopt(m_socket, level, optname, optval, optlen);
}

//! Host

struct hostent* ccSocket::GetHostByName(const std::string& name)
{
    return gethostbyname(name.c_str());
}

//! Server
bool ccSocket::Listen(const uint32_t& backlog)
{
    return Listen(m_ip, m_port, backlog);
}

bool ccSocket::Listen(const std::string& ip, const uint32_t& port, const uint32_t& backlog)
{
    assert(m_socket != -1 && "socket is unavailable");
    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    TRACE("ip:", ip);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    int bind_ret = bind(m_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    TRACE("bind ret == ", bind_ret);
    int listen_ret = listen(m_socket, backlog);
    TRACE("listen ret == ", listen_ret);
    return bind_ret == 0 && listen_ret == 0;
}


bool ccSocket::Accept(ccSocket& client)
{
    socklen_t clntAddrSize;
    sockaddr_in clntAddr;
    clntAddrSize = sizeof(clntAddr);
    int socket = accept(m_socket, (struct sockaddr *) &clntAddr, &clntAddrSize);
    if (socket != -1)
    {
        client.SetSocket(socket);
    }
    return socket != -1;
}

//! Client

bool ccSocket::Connect(const std::string& ip, const uint32_t& port)
{
    m_ip = ip;
    m_port = port;
    return Connect();
}

bool ccSocket::Connect()
{
    assert(m_socket != -1 && "socket is unavailable");
    assert(!m_ip.empty() && "address is empty");
    TRACE("Connect to ", m_ip, ":", m_port);

    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    server_addr.sin_port = htons(m_port);

    int rc = connect(m_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    return rc == 0;
}

void ccSocket::Close()
{
    if (m_socket != -1)
    {
        shutdown(m_socket, SHUT_RDWR);
        close(m_socket);
    }
    m_socket = -1;
}

//! I/O

SocketIOInfo ccSocket::Send(const std::string& data)
{
    return Send(data.c_str(), data.size());
}

SocketIOInfo ccSocket::Send(const void* buffer, const size_t& len)
{
    assert(m_socket != -1 && "unavailable socket");
    SocketIOInfo result;
    const char* buffer_ptr = static_cast<const char*>(buffer);
    size_t nleft = len;
    ssize_t n = 0;
    ssize_t nwrite = 0;
    do
    {
        n = write(m_socket, buffer_ptr + nwrite, nleft);
        TRACE("write data :", n);
        if (n > 0)
        {
            nwrite += n;
            nleft -= n;
            TRACE("left : ", nleft);
        }
    } while(nleft > 0 && n > 0);

    result.m_count = nwrite;
    result.m_error = n == -1? errno : 0;
    m_send_count++;
    TRACE("send count :", m_send_count);
    return result;
}

SocketIOInfo ccSocket::Recv(void* buffer, const size_t& len)
{
    assert(m_socket != -1 && "unavailable socket");
    SocketIOInfo result;
    char* buffer_ptr = static_cast<char*>(buffer);
    bzero(buffer_ptr, len);
    size_t nleft = len;
    ssize_t n = 0;
    ssize_t nread = 0;
    do
    {
        n = read(m_socket, buffer_ptr + nread, nleft);
        TRACE("read data : ", n);
        if (n > 0)
        {
            nleft -= n;
            nread += n;
            TRACE("left :", nleft);
        }
    } while(nleft > 0 && n > 0);
    result.m_count = nread;
    result.m_error = n == -1? errno : 0;
    m_recv_count++;
    TRACE("recv count :", m_recv_count);
    return result;
}

} /* namespace cc */
