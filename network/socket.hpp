#pragma once
#include "epoll/epoll.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "coroutine.h"

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
    address() : len_(sizeof(sockaddr_storage)) {
        bzero(&addr_, sizeof(addr_));
    }
    address(const std::string &host, std::uint16_t port)
        : host_(host), port_(std::to_string(port))
    {
    }
    address(const struct sockaddr_storage *sockaddr, socklen_t len) {
      len_ = len;
      memcpy(&addr_, sockaddr, len);
    }

    std::string ip() const
    {
        char buf[sizeof("255.255.255.255")] = {0};
        return std::string(inet_ntop(AF_INET, &((const struct sockaddr_in *)&addr_)->sin_addr, buf, sizeof(buf)));
    }

    uint16_t port() const
    {
        return ntohs(((struct sockaddr_in *)&addr_)->sin_port);
    }

    uint32_t ipv4() const {
      return ((struct sockaddr_in *)&addr_)->sin_addr.s_addr;
    }

    int resolve()
    {
        struct addrinfo *info;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;

        int ret = getaddrinfo(host_.c_str(), port_.c_str(), &hints, &info);
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
    bool operator <(const address &addr) const
    {
        if (len_ < addr.len_) {
            return true;
        } else if (len_ > addr.len_) {
            return false;
        }
        return memcmp(&addr_, &addr.addr_,  len_) < 0;
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

    udp_socket(udp_socket &&sock) noexcept : async_file_event(std::move(sock))
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
        if (::bind(handle_.fd_, addr.get_sockaddr(), addr.get_sockaddr_len()) < 0)
        {
            perror("bind");
        }
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
            else if ((size_t)ret != buffer.size())
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


class tcp_socket: public async_file_event
{
public:
    tcp_socket(const tcp_socket &) = delete;

    tcp_socket(epoll_event_pool &pool) : async_file_event(pool, socket(AF_INET, SOCK_STREAM, 0))
    {
    }

    tcp_socket(tcp_socket &&sock) noexcept : async_file_event(std::move(sock))
    {
        handle_.fd_ = sock.handle_.fd_;
        sock.handle_.fd_ = -1;
    }

    ~tcp_socket()
    {
        if (handle_.fd_ != -1) {
            close(handle_.fd_);
        }
    }
    void bind(address &addr)
    {
        if (::bind(handle_.fd_, addr.get_sockaddr(), addr.get_sockaddr_len()) < 0)
        {
          throw std::runtime_error("bind failed");
        }
    }

    int listen(int num) { return ::listen(handle_.fd_, num); }

    address get_local_address() const {
      struct sockaddr_storage storage;
      socklen_t len = sizeof(storage);
      int ret = getsockname(handle_.fd_, (struct sockaddr *)&storage, &len);
      if (ret) {
        throw std::runtime_error("getsockname failed");
      }
      return address(&storage, len);
    }

    tcp_socket accept() {
      int fd = -1;
      do {
        fd = ::accept(handle_.fd_, NULL, NULL);
        if (fd == -1) {
          if (errno != EAGAIN) {
            throw std::runtime_error("accept failed");
          }
        } else {
          break;
        }
        waited_ |= CAN_READ;
        co_yield(&co_read);
        waited_ &= ~CAN_READ;
      } while (1);
      tcp_socket sock(pool_);
      sock.handle_.fd_ = fd;
      return sock;
    }

    int connect(const address &addr)
    {
        int ret;
        if ((ret = ::connect(handle_.fd_, addr.get_sockaddr(), addr.get_sockaddr_len()) < 0))
        {
            if (ret < 0)
            {
                if (errno == EINPROGRESS)
                {
                  waited_ |= CAN_WRITE;
                  co_yield(&co_write);
                  waited_ &= ~CAN_WRITE;

                  int so_error;
                  socklen_t option_len;
                  if (getsockopt(handle_.fd_, SO_ERROR, SOL_SOCKET, &so_error,
                                 &option_len)) {
                    return -1;
                    }
                    if (so_error == 0) {
                        return 0;
                    }
                }
            }
        }
        return -1;
    }
};

