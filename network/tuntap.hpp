#pragma once

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

#include <cinttypes>

#include <stdexcept>


class tuntap
{
public:
    tuntap()
    {
        fd_ = open("/dev/net/tun", O_RDWR);
        if (fd_ < 0)
        {
            throw std::runtime_error("unable to open /dev/net/tun");
        }
    }
    virtual ~tuntap()
    {
        if (close(fd_) < 0)
        {
            perror("close");
        }
    }
    int fd() const
    {
        return fd_;
    }
protected:
    int fd_;
};


class tap : public tuntap
{
public:
    tap(const std::string &name)
    {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
        ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
        if (ioctl(fd_, TUNSETIFF, (void *)&ifr) < 0)
        {
            throw std::runtime_error("ioctl failed");
        }
    }
    virtual ~tap()
    {}
};

class tun : public tuntap
{
public:
    tun(const std::string &name)
    {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);
        ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
        if (ioctl(fd_, TUNSETIFF, (void *)&ifr) < 0)
        {
            throw std::runtime_error("ioctl failed");
        }
    }
    virtual ~tun()
    {}
};
