#include "bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"
#include "bridge.hpp"
#include "bytebuffer.hpp"
#include "AES.hpp"

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
        byte_buffer buffer(ethernet_bridge::bridge_mtu);
        int ret = event->async_read_some(buffer, buffer.size());
        if (ret < 0)
        {
            perror("read\n");
            abort();
        }
        printf("%d from tap\n", ret);

        buffer.resize(ret);
        bridge.forward(buffer, reply_handler);
    }
}

static void read_udp(void *p)
{
    udp_socket *sock = (udp_socket *)p;
    address addr("0.0.0.0", 9999);
    addr.resolve();
    sock->bind(addr);
    AES aes(AES_key("my passwd"));
    printf("listening on 0.0.0.0:9999\n");

    while (1)
    {
        address addr;
        byte_buffer buffer(ethernet_bridge::bridge_mtu);

        int ret = sock->async_read_from(addr, buffer);
        if (ret < 0)
        {
            perror("async_send_to");
        }
        printf("%d from udp\n", ret);

        buffer.resize(ret);
        xor_mess(buffer, buffer.size());
        byte_buffer decrypt_buffer = aes.AES_decrypt(buffer);
        bridge.forward(decrypt_buffer, [&](const byte_buffer &buffer)
        {
            address addr_copy = addr;
            byte_buffer encrypt_buffer = aes.AES_encrypt(buffer);
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
    coroutine_t *co = co_create(65536, (void *)read_tap, &event);
    co_post(co);

    udp_socket sock(pool);
    co = co_create(65536, (void *)read_udp, &sock);
    co_post(co);

    pool.poll_forever();

    return 0;
}
