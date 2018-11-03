#include "bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"
#include "bridge.hpp"

ethernet_bridge bridge;

static void read_tap(void *p)
{
    async_file_event *event = (async_file_event *)p;
    auto reply_handler = [&](const byte_buffer &buffer)
    {
        event->async_write(buffer, buffer.size());
    };

    while (1)
    {
        char buf[1500];
        int ret = event->async_read_some(buf, 1500);
        if (ret < 0)
        {
            perror("read\n");
            abort();
        }
        printf("%d from tap\n", ret);

        byte_buffer buffer(buf, ret);
        bridge.forward(buffer, reply_handler);
    }
}

static void read_udp(void *p)
{
    udp_socket *sock = (udp_socket *)p;
//    async_timer_event timer(sock->pool());


//    address addr("180.76.76.76", 53);
//    addr.resolve();
//    char query[] = "\x12\x34\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x05\x62\x61\x69\x64\x75\x03\x63\x6f\x6d\x00\x00\x01\x00\x01";

//    byte_buffer buffer(sizeof(query) - 1);
//    buffer.fill(query, sizeof(query) - 1);

//    address addr2;
//    byte_buffer buffer2(1500);
    address addr("0.0.0.0", 9999);
    addr.resolve();
    sock->bind(addr);
    printf("listening on 0.0.0.0:9999\n");

    while (1)
    {
        address addr;
        byte_buffer buffer(1500);

        int ret = sock->async_read_from(addr, buffer);
        if (ret < 0)
        {
            perror("async_send_to");
        }
        xor_mess(buffer, ret);
        printf("%d from udp\n", ret);

        buffer.resize(ret);
        bridge.forward(buffer, [&](const byte_buffer &buffer)
        {
            address addr_copy = addr;
            byte_buffer encrypt_buffer(buffer);
            xor_mess(encrypt_buffer, encrypt_buffer.size());
            sock->async_send_to(addr_copy, encrypt_buffer);
        });
    }
}

int server()
{
    epoll_event_pool pool;

    int tap_fd = open_tap();
    fd_handle handle(tap_fd);
    async_file_event event(pool, handle);
    coroutine_t *co = co_create(4096, (void *)read_tap, &event);
    co_post(co);

    udp_socket sock(pool);
    co = co_create(16384, (void *)read_udp, &sock);
    co_post(co);

    pool.poll_forever();

    return 0;
}
