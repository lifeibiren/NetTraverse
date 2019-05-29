#include "ether_bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"

#include "AES.hpp"

#include "config.hpp"

struct client_env {
    async_file_event *tap;
    udp_socket *udp;
    address addr;
    AES *aes;
};

static void read_tap(client_env &env)
{
    async_file_event *event = env.tap;
    udp_socket *sock = env.udp;
    AES *aes = env.aes;

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
        sock->async_send_to(env.addr, encrypt_buffer);
    }
}

static void read_udp(client_env &env)
{
    async_file_event *event = env.tap;
    udp_socket *sock = env.udp;
    AES *aes = env.aes;

    while (1)
    {
        address addr;
        byte_buffer buffer(ethernet_bridge::bridge_mtu);

        int ret = sock->async_read_from(addr, buffer);
        if (ret < 0)
        {
            perror("async_send_to");
            continue;
        }
        printf("%d from udp\n", ret);

        buffer.resize(ret);
        xor_mess(buffer, buffer.size());
        byte_buffer decrypt_buffer = aes->AES_decrypt(buffer);
        event->async_write(decrypt_buffer, decrypt_buffer.size());
    }
}

int client(const ether_bridge_conf &conf) {
  address server_addr(conf.remote_addr_, conf.remote_port_);
  server_addr.resolve();

  epoll_event_pool pool;

  tap tap0(conf.tap_device_);
  fd_handle handle(tap0.fd());
  async_file_event event(pool, handle);
  udp_socket sock(pool);

  client_env env;
  env.tap = &event;
  env.udp = &sock;
  env.addr = server_addr;
  env.aes = new AES(AES_key(conf.key_));

  coroutine *co = co_create_cxx_shared(std::bind(read_tap, std::ref(env)));
  co_post(co);

  co = co_create_cxx_shared(std::bind(read_udp, std::ref(env)));
  co_post(co);

  pool.poll_forever();

  return 0;
}
