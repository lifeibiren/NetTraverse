#include "bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"
#include "bridge.hpp"

#include "AES.hpp"

struct client_env {
    async_file_event *tap;
    udp_socket *udp;
    address addr;
    AES *aes;
};
static void read_tap(void *p)
{
    async_file_event *event = ((client_env *)p)->tap;
    udp_socket *sock = ((client_env *)p)->udp;
    AES *aes = ((client_env *)p)->aes;


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
        byte_buffer encrypt_buffer = aes->AES_encrypt(buffer);
        xor_mess(encrypt_buffer, encrypt_buffer.size());
        sock->async_send_to(((client_env *)p)->addr, encrypt_buffer);
    }
}

static void read_udp(void *p)
{
    async_file_event *event = ((client_env *)p)->tap;
    udp_socket *sock = ((client_env *)p)->udp;
    AES *aes = ((client_env *)p)->aes;

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
        byte_buffer decrypt_buffer = aes->AES_decrypt(buffer);
        event->async_write(decrypt_buffer, decrypt_buffer.size());
    }
}

int client(address server_addr)
{
    server_addr.resolve();

    epoll_event_pool pool;

    int tap_fd = open_tap();
    fd_handle handle(tap_fd);
    async_file_event event(pool, handle);
    udp_socket sock(pool);

    client_env env;
    env.tap = &event;
    env.udp = &sock;
    env.addr = server_addr;
    env.aes = new AES(AES_key("my passwd"));

    coroutine_t *co = co_create(65536, (void *)read_tap, &env);
    co_post(co);

    co = co_create(65536, (void *)read_udp, &env);
    co_post(co);

    pool.poll_forever();

    return 0;
}
