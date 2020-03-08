#ifndef CCSOCKET_H_
#define CCSOCKET_H_

#include <string>
#include <arpa/inet.h>
#include <netdb.h>

namespace cc
{

struct SocketIOInfo
{
    SocketIOInfo() : m_count(0), m_error(0) {}

    void Reset()
    {
        m_count = 0;
        m_error = 0;
    }

    size_t m_count;
    int m_error;

};

class ccSocket
{
public:
    ccSocket(const ccSocket&)=delete;
    ccSocket& operator=(ccSocket&)=delete;

    ccSocket();

    virtual ~ccSocket();

    //! Socket
    void InitSocket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);

    void SetSocket(int socket);

    int GetSocket();

    //! IP Address
    void SetIP(const std::string& ip);

    const std::string& GetIP() const;

    //! Port
    void SetPort(const uint32_t& port);

    uint32_t GetPort();

    //! Opt
    int SetSockOpt(const int& level, const int& optname, const void *optval, const socklen_t& optlen);

    int GetSockOpt(const int& level, const int& optname, void *optval, socklen_t* optlen);

    //! Host
    struct hostent *GetHostByName(const std::string& name);

    //! Server
    bool Listen(const uint32_t& backlog);

    bool Listen(const std::string& ip, const uint32_t& port, const uint32_t& backlog = 500);

    bool Accept(ccSocket& client);

    //! Client
    bool Connect(const std::string& ip, const uint32_t& port);

    bool Connect();

    void Close();

    //! I/O
    SocketIOInfo Send(const std::string& data);

    SocketIOInfo Send(const void* buffer, const size_t& len);

    SocketIOInfo Recv(void* buffer, const size_t& len);

private:
    int m_socket;
    std::string m_ip;
    uint32_t m_port;
    int m_recv_count;
    int m_send_count;
};

} /* namespace cc */

#endif /* CCSOCKET_CCSOCKET_H_ */
