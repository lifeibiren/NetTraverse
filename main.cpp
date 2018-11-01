#include "epoll.hpp"
#include "socket.hpp"

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void test_co(void *p)
{
    async_file_event *event = (async_file_event *)p;
    while (1)
    {
        char buf[1500];
        int ret = event->async_read_some(buf, 1500);
        if (ret < 0)
        {
            perror("read\n");
            abort();
        }
        printf("%d\n", ret);
    }
}

void dns(void *p)
{
    udp_socket *sock = (udp_socket *)p;
    address addr("180.76.76.76", 53);
    addr.resolve();
    char query[] = "\x12\x34\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x05\x62\x61\x69\x64\x75\x03\x63\x6f\x6d\x00\x00\x01\x00\x01";

    byte_buffer buffer(sizeof(query) - 1);
    buffer.fill(query, sizeof(query) - 1);

    address addr2;
    byte_buffer buffer2(1500);
    while (1)
    {
        int ret = sock->async_send_to(addr, buffer);
        if (ret < 0)
        {
            perror("async_send_to");
        }
        ret = sock->async_read_from(addr2, buffer2);
        std::cout<<"DNS query received "<<ret<<" bytes"<<std::endl;
        //std::cout<<(char *)(void *)buffer2<<std::endl;
    }
}

int main(int argc, char *args[])
{
    co_init();

    setvbuf(stdout, NULL, _IONBF, 0);
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

    epoll_event_pool pool;

    fd_handle handle(fd);
//    pool.add_event(file_event(handle, [](epoll_event_pool &pool, fd_handle &handle){
//                       std::uint8_t buf[2000];
//                       int length;
//                       do {
//                           length = read(handle.fd_, buf, 2000);
//                           printf("%d\n", length);
//                       } while (length < 0);
//                       std::cout<<"triggered"<<std::endl;
//                   }));
    async_file_event event(pool, handle);
    coroutine_t *co = co_create(4096, (void *)test_co, &event);
    co_post(co);

    udp_socket sock(pool);
    co = co_create(16384, (void *)dns, &sock);
    co_post(co);
//    pool.add_event();

//            for (int n = 0; n < nfds; ++n) {
//                if (events[n].data.fd == listen_sock) {
//                    struct sockaddr_in addr;
//                    unsigned int addrlen = sizeof(addr);
//                    conn_sock = accept(listen_sock,
//                                       (struct sockaddr *) &addr, &addrlen);
//                    if (conn_sock == -1) {
//                        perror("accept");
//                        exit(EXIT_FAILURE);
//                    }
//                    set_non_block(conn_sock);
//                    ev.events = EPOLLIN | EPOLLET;
//                    ev.data.fd = conn_sock;
//                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
//                                  &ev) == -1) {
//                        perror("epoll_ctl: conn_sock");
//                        exit(EXIT_FAILURE);
//                    }
//                } else {
//                    do_use_fd(events[n].data.fd);
//                }
//            }

    pool.poll_forever();
    return 0;
}
