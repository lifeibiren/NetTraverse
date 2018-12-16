#include "udp_bridge.hpp"

#include "coroutine.h"
#include "config.hpp"
#include "event.hpp"
#include "socket.hpp"
#include <functional>
#include "bytebuffer.hpp"

static udp_bridge bridge;

void udp_dispather(config &conf, epoll_event_pool &pool)
{
    udp_socket sock(pool);
    address addr("192.168.3.3", 3333);
    addr.resolve();

    sock.bind(addr);

    while (1)
    {
        address addr;
        byte_buffer buffer(udp_bridge::udp_mtu);

        int ret = sock.async_read_from(addr, buffer);
        if (ret < 0)
        {
            perror("async_send_to");
        }
        printf("%d from udp\n", ret);

        buffer.resize(ret);

        bridge.foward(sock, buffer, addr);
//        xor_mess(buffer, buffer.size());
//        byte_buffer decrypt_buffer = aes.AES_decrypt(buffer);
//        bridge.forward(decrypt_buffer, [&](const byte_buffer &buffer)
//        {
//            address addr_copy = addr;
//            byte_buffer encrypt_buffer = aes.AES_encrypt(buffer);
//            xor_mess(encrypt_buffer, encrypt_buffer.size());
//            sock.async_send_to(addr_copy, encrypt_buffer);
//        });
    }
}

int udp_bridge_main(config &conf)
{
    epoll_event_pool pool;
    coroutine *co = co_create_cxx(std::bind(udp_dispather, std::ref(conf), std::ref(pool)), 65536);
    co_post(co);

    pool.poll_forever();
    return 0;
}

