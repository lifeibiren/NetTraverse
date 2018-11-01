#pragma once
#include "epoll.hpp"
#include <sys/timerfd.h>

#include <iostream>
namespace posix_time
{
    class time
    {
    public:
        time(time_t sec, long nsec) :
            ts({sec, nsec})
        {}
        bool operator <(const time &r_val) const
        {
            if (ts.tv_sec < r_val.ts.tv_sec)
                return true;
            else if (ts.tv_sec > r_val.ts.tv_sec)
                return false;
            else if (ts.tv_nsec < r_val.ts.tv_nsec)
                return true;
            else
                return false;
        }
        operator struct timespec &()
        {
            return ts;
        }
        operator const struct timespec &() const
        {
            return ts;
        }
    protected:
        struct timespec ts;
    };


    class seconds : public time
    {
    public:
        seconds(time_t sec) : time(sec, 0) {}
    };
}

class async_timer_event : protected async_file_event
{
public:
    async_timer_event(epoll_event_pool &pool) :
        async_file_event(pool, fd_handle(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)))
    {
    }
    ~async_timer_event()
    {
//        co_destroy(co_wait);
    }
    template<typename T>
    void expire_from_now(const T &time)
    {
        struct timespec ts = time;

        struct itimerspec new_time;
        bzero(&new_time, sizeof(new_time));
        new_time.it_value = ts;
        if (timerfd_settime(handle_.fd_, 0, &new_time, NULL) < 0)
            perror("timerfd_settime");
    }
    void async_wait()
    {
        uint64_t count;
        async_read(&count, sizeof(count));
        if (count == 0)
        {
            std::cout<<"a read of timerfd returned zero";
        }
    }
};
