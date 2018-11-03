#pragma once

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

#include <cinttypes>

static inline int open_tap()
{
    int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        perror("open");
    }
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "tap0", IFNAMSIZ);
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    ioctl(fd, TUNSETIFF, (void *)&ifr);

    printf("%s\n", ifr.ifr_name);
    return fd;
}

static inline void xor_mess(void *buf, std::size_t length)
{
    std::uint8_t *bytes = (std::uint8_t *)buf;
    for (std::size_t i = 0; i < length; i++)
    {
        bytes[i] ^= 0x0f;
    }
}
