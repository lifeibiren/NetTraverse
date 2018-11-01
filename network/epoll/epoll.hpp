#pragma once
#include "event.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <map>
#include <functional>

#include "coroutine.h"

class fd_handle
{
public:
    fd_handle() : fd_(-1) {}
    fd_handle(int fd) : fd_(fd){}
    fd_handle(const fd_handle &handle) : fd_(handle.fd_) {}
    int set_non_block(void)
    {
        int ret = fcntl(fd_, F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            perror("fcntl");
            return ret;
        }
        return 0;
    }

    int get_fd() const
    {
        return fd_;
    }
    int fd_;
};

class fd_event_handler
{
public:
    fd_event_handler(const fd_event_handler &)
    {

    }
    void operator() (event_handle &handle)
    {
    }
};

class epoll_event_pool;
enum event_type
{
    CAN_READ = 0x1,
    CAN_WRITE = 0x2,
    ERROR = 0x4,
    EVNET_MASK = CAN_READ | CAN_WRITE | ERROR
};

class file_event
{
public:
//    file_event(fd_handle &handle, fd_event_handler &handler) :
//        handle_(handle), handler_(handler) {}
    typedef std::function<void(epoll_event_pool &, fd_handle &)> file_event_handler_type;
    file_event(const fd_handle &handle, file_event_handler_type handler) :
        handle_(handle), handler_function_(handler) {}
    void trigger(epoll_event_pool &pool, int event_type)
    {
//        handler_(handle_);
        handler_function_(pool, handle_);
    }
protected:
    friend class epoll_event_pool;
    fd_handle handle_;
//    fd_event_handler handler_;
    file_event_handler_type handler_function_;
};

class epoll_event_pool
{
public:

    typedef std::function<void (epoll_event_pool &pool, int event)> file_event_handler_type;
    epoll_event_pool()
    {
        epollfd = epoll_create1(0);
        if (epollfd == -1) {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }
    }

    void add_event(fd_handle &handle, file_event_handler_type handler)
    {
        int fd = handle.fd_;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.fd = fd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }
        handle_map.emplace(std::make_pair(handle, handler));
    }

    void del_event(event &handle)
    {
        handle_map.empty();
    }

    void clear_event()
    {
        handle_map.clear();
    }

    void poll_forever()
    {
        for (;;) {
            while (co_num_ready() > 1) {
                co_resched();
            }
            nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                perror("epoll_wait");
//                exit(EXIT_FAILURE);
            }
            for (int n = 0; n < nfds; ++n) {
                fd_handle handle(events[n].data.fd);
                auto it = handle_map.find(handle);
                if (it != handle_map.end())
                {
                    int event = 0;
                    if (events[n].events | EPOLLIN)
                        event |= CAN_READ;
                    if (events[n].events | EPOLLOUT)
                        event |= CAN_WRITE;
                    it->second(*this, event);
                }
            }
        }
    }
    void poll_one() {}
protected:
    enum{ MAX_EVENTS = 10 };
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;

    struct fd_handle_comp
    {
        bool operator()(const fd_handle &h1, const fd_handle &h2)
        {
            return h1.fd_ < h2.fd_;
        }
    };

    std::map<fd_handle, file_event_handler_type, fd_handle_comp> handle_map;
};



class async_file_event
{
protected:
    epoll_event_pool &pool_;
    fd_handle handle_;
    int event_;
    int waited_;
    int waited_mask_;
    coroutine_t *co_read, *co_write, *co_wait;
public:
    async_file_event(epoll_event_pool &pool, fd_handle h) : pool_(pool), handle_(h), event_(0), waited_(0), waited_mask_(0),
        co_read(NULL), co_write(NULL), co_wait(NULL)
    {
        pool.add_event(handle_, std::bind(&async_file_event::trigger, this, std::placeholders::_1, std::placeholders::_2));
        handle_.set_non_block();
    }
    async_file_event(async_file_event &&event) : pool_(event.pool_), handle_(event.handle_)
    {

    }

    epoll_event_pool &pool()
    {
        return pool_;
    }

    int async_wait(int event)
    {
        if (event_ & event)
        {
            return 0;
        }
        waited_mask_ = event;
        co_yield(&co_wait);
        co_wait = NULL;
        waited_mask_ = 0;
        return event_;
    }
    int async_read_some(void *buf, size_t count)
    {
        ssize_t ret;
        size_t offset = 0;
        do {
            ret = read(handle_.fd_, (std::uint8_t *)buf + offset, count - offset);
            if (ret > 0)
            {
                offset += ret;
            }
            if (offset == count)
            {
                return offset;
            }
            if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    if (offset > 0)
                    {
                        return offset;
                    }
                    else
                    {
                        waited_ |= CAN_READ;
                        co_yield(&co_read);
                        co_read = NULL;
                        waited_ &= ~CAN_READ;
                    }
                }
                else
                {
                    break;
                }
            }
        } while(1);
        return ret;
    }
    int async_write_some(void *buf, size_t count)
    {
        ssize_t ret;
        size_t offset = 0;
        do {
            ret = write(handle_.fd_, (std::uint8_t *)buf + offset, count - offset);
            if (ret > 0)
            {
                offset += ret;
            }
            if (offset == count)
            {
                return offset;
            }
            if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    if (offset > 0)
                    {
                        return offset;
                    }
                    else
                    {
                        waited_ |= CAN_WRITE;
                        co_yield(&co_write);
                        co_write = NULL;
                        waited_ &= ~CAN_WRITE;
                    }
                }
                else
                {
                    break;
                }
            }
        } while(1);
        return ret;
    }
    int async_read(void *buf, size_t count)
    {
        ssize_t ret;
        size_t offset = 0;
        do {
            ret = read(handle_.fd_, (std::uint8_t *)buf + offset, count - offset);
            if (ret > 0)
            {
                offset += ret;
            }
            if (offset == count)
            {
                return offset;
            }
            if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    waited_ |= CAN_READ;
                    co_yield(&co_read);
                    co_read = NULL;
                    waited_ &= ~CAN_READ;
                }
                else
                {
                    break;
                }
            }
        } while(1);
        return ret;
    }
    int async_write(void *buf, size_t count)
    {
        ssize_t ret;
        size_t offset = 0;
        do {
            ret = write(handle_.fd_, (std::uint8_t *)buf + offset, count - offset);
            if (ret > 0)
            {
                offset += ret;
            }
            if (offset == count)
            {
                return offset;
            }
            if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    waited_ |= CAN_WRITE;
                    co_yield(&co_write);
                    co_write = NULL;
                    waited_ &= ~CAN_WRITE;
                }
                else
                {
                    break;
                }
            }
        } while(1);
        return ret;
    }
    void trigger(epoll_event_pool &pool, int event)
    {
        event_ |= event;
        if (waited_ & event_ & CAN_WRITE)
        {
            co_post(co_write);
        }
        if (waited_ & event_ & CAN_READ)
        {
            co_post(co_read);
        }
        if (waited_mask_ & event_)
        {
            co_post(co_wait);
        }
    }
};
