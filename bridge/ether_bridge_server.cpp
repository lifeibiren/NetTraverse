#include "ether_bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"
#include "bytebuffer.hpp"
#include "AES.hpp"

#include "config.hpp"

ethernet_bridge bridge;

static void read_tap(config &conf, epoll_event_pool &pool)
{
    tap tap0(conf.tap_dev());
    fd_handle handle(tap0.fd());
    async_file_event event(pool, handle);

    auto reply_handler = [&](const byte_buffer &buffer)
    {
        event.async_write(buffer, buffer.size());
    };

    while (1)
    {
        byte_buffer buffer(ethernet_bridge::bridge_mtu);
        int ret = event.async_read_some(buffer, buffer.size());
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

static void read_udp(config &conf, epoll_event_pool &pool)
{
    udp_socket sock(pool);

    address addr(conf.bind_address(), conf.bind_port());
    addr.resolve();
    sock.bind(addr);
    AES aes(AES_key(conf.key()));
    std::cout<<"listening on "<<conf.bind_address()<<":"<<conf.bind_port()<<std::endl;

    while (1)
    {
        address addr;
        byte_buffer buffer(ethernet_bridge::bridge_mtu);

        int ret = sock.async_read_from(addr, buffer);
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
            sock.async_send_to(addr_copy, encrypt_buffer);
        });
    }
}

int server(config &conf)
{
    epoll_event_pool pool;
    coroutine *co = co_create_cxx(std::bind(read_tap, std::ref(conf), std::ref(pool)), 65536);
    co_post(co);

    co = co_create_cxx(std::bind(read_udp, std::ref(conf), std::ref(pool)), 65536);
    co_post(co);

    pool.poll_forever();

    return 0;
}
