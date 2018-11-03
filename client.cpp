#include "bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"
#include "bridge.hpp"

struct client_env {
    async_file_event *tap;
    udp_socket *udp;
    address addr;
};
static void read_tap(void *p)
{
    async_file_event *event = ((client_env *)p)->tap;
    udp_socket *sock = ((client_env *)p)->udp;

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
        xor_mess(buffer, buffer.size());
        sock->async_send_to(((client_env *)p)->addr, buffer);
    }
}

static void read_udp(void *p)
{
    async_file_event *event = ((client_env *)p)->tap;
    udp_socket *sock = ((client_env *)p)->udp;

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
        event->async_write(buffer, ret);
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

    coroutine_t *co = co_create(16384, (void *)read_tap, &env);
    co_post(co);

    co = co_create(16384, (void *)read_udp, &env);
    co_post(co);

    pool.poll_forever();

    return 0;
}
