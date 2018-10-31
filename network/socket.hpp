#include "epoll/epoll.hpp"

#include "coroutine.h"
#include <unistd.h>

class address
{
public:
    const struct sockaddr *get_sockaddr() {}
    socklen_t get_sockaddr_len() {}
};

class byte_buffer
{

};

class udp_socket
{
protected:
    fd_handle handle_;
    epoll_event_pool &pool_;
public:
    udp_socket(epoll_event_pool &pool) : handle_(socket(AF_INET, SOCK_DGRAM, 0)), pool_(pool)
    {

    }
    udp_socket(const udp_socket &sock) : pool_(sock.pool_)
    {
        handle_.fd_ = dup(sock.handle_.fd_);
    }
    udp_socket(udp_socket &&sock) : pool_(sock.pool_)
    {
        handle_.fd_ = sock.handle_.fd_;
        sock.handle_.fd_ = -1;
    }
    ~udp_socket()
    {
        if (handle_.fd_ != -1) {
            close(handle_.fd_);
        }
    }
    void bind(address &addr)
    {
        ::bind(handle_.fd_, addr.get_sockaddr(), addr.get_sockaddr_len());
    }
    int async_send_to(address &addr, const byte_buffer &buffer)
    {

    }
    int async_read_from(address &addr, byte_buffer &buffer)
    {

    }
    int connect(const address &addr)
    {

    }
};
