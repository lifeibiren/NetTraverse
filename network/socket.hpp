#pragma once
#include "epoll/epoll.hpp"

#include "coroutine.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>

#include "bytebuffer.hpp"

class address
{
public:
    const struct sockaddr *get_sockaddr() const
    {
        return (const struct sockaddr *)&addr_;
    }
    const socklen_t &get_sockaddr_len() const
    {
        return len_;
    }
    struct sockaddr *get_sockaddr()
    {
        return (struct sockaddr *)&addr_;
    }
    socklen_t &get_sockaddr_len()
    {
        return len_;
    }
    address() {}
    address(const std::string &host, std::uint16_t port)
        : host_(host), port_(std::to_string(port))
    {
    }

    int resolve()
    {
        struct addrinfo *info;
        int ret = getaddrinfo(host_.c_str(), port_.c_str(), NULL, &info);
        if (ret < 0)
        {
            perror("getaddrinfo");
            return ret;
        }
        else
        {
            bcopy(info->ai_addr, &addr_, info->ai_addrlen);
            len_ = info->ai_addrlen;
        }
        freeaddrinfo(info);
        return ret;
    }

    virtual ~address()
    {
    }
protected:
    std::string host_;
    std::string port_;
    struct sockaddr_storage addr_;
    socklen_t len_;
};

class udp_socket : public async_file_event
{
public:
    udp_socket(const udp_socket &sock) = delete;

    udp_socket(epoll_event_pool &pool) : async_file_event(pool, socket(AF_INET, SOCK_DGRAM, 0))
    {
    }

    udp_socket(udp_socket &&sock) : async_file_event(std::move(sock))
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
    int async_send_to(const address &addr, const byte_buffer &buffer)
    {
        ssize_t ret;
        do {
            ret = sendto(handle_.fd_, buffer, buffer.size(), 0, addr.get_sockaddr(), addr.get_sockaddr_len());
            if (ret < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    waited_ |= CAN_WRITE;
                    co_yield(&co_write);
                    waited_ &= ~CAN_WRITE;
                }
                else
                {
                    perror("sendto");
                    return ret;
                }
            }
            else if (ret != buffer.size())
            {
                perror("sendto");
                return ret;
            }
            else
            {
                return ret;
            }
        } while(1);
        return 0;
    }
    int async_read_from(address &addr, byte_buffer &buffer)
    {
        ssize_t ret;
        do {
            ret = recvfrom(handle_.fd_, buffer, buffer.size(), 0, addr.get_sockaddr(), &addr.get_sockaddr_len());
            if (ret < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    waited_ |= CAN_READ;
                    co_yield(&co_read);
                    waited_ &= ~CAN_READ;
                }
                else
                {
                    perror("sendto");
                    return ret;
                }
            }
            else
            {
                return ret;
            }
        } while(1);
        return 0;
    }
    int connect(const address &addr)
    {
        return false;
    }
};
