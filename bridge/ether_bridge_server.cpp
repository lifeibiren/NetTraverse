#include "ether_bridge.hpp"

#include "epoll.hpp"
#include "tuntap.hpp"
#include "coroutine.h"
#include "timer.hpp"
#include "bytebuffer.hpp"
#include "AES.hpp"

#include <memory>

#include "config.hpp"

ethernet_bridge bridge;

static void read_tap(const ether_bridge_conf &conf, epoll_event_pool &pool) {
  tap tap0(conf.tap_device_);
  fd_handle handle(tap0.fd());
  async_file_event event(pool, handle);

  auto reply_handler = [&](const byte_buffer &buffer) {
    event.async_write(buffer, buffer.size());
  };

  while (1) {
    byte_buffer buffer(ethernet_bridge::bridge_mtu);
    int ret = event.async_read_some(buffer, buffer.size());
    if (ret < 0) {
      perror("read\n");
      abort();
    }
    printf("%d from tap\n", ret);

    buffer.resize(ret);
    bridge.forward(buffer, reply_handler);
  }
}

static void read_udp(const ether_bridge_conf &conf, epoll_event_pool &pool) {
  udp_socket sock(pool);

  address addr(conf.bind_addr_, conf.bind_port_);
  addr.resolve();
  sock.bind(addr);
  AES aes(AES_key(conf.key_));
  std::cout << "listening on " << conf.bind_addr_ << ":" << conf.bind_port_
            << std::endl;

  while (1) {
    address addr;
    byte_buffer buffer(ethernet_bridge::bridge_mtu);

    int ret = sock.async_read_from(addr, buffer);
    if (ret < 0) {
      perror("async_send_to");
    }
    printf("%d from udp\n", ret);

    buffer.resize(ret);
    xor_mess(buffer, buffer.size());
    byte_buffer decrypt_buffer = aes.AES_decrypt(buffer);
    bridge.forward(decrypt_buffer, [&](const byte_buffer &buffer) {
      address addr_copy = addr;
      byte_buffer encrypt_buffer = aes.AES_encrypt(buffer);
      xor_mess(encrypt_buffer, encrypt_buffer.size());
      sock.async_send_to(addr_copy, encrypt_buffer);
    });
  }
}

static void socks5_handler(epoll_event_pool &pool,
                           std::shared_ptr<tcp_socket> sock) {
  std::cout << "receive socks5 handshake\n";

  uint8_t buf[2];
  sock->async_read(buf, 2);

  uint8_t len = buf[1];
  uint8_t vers[len];
  sock->async_read(vers, len);

  uint8_t ver_riplies[2] = {0x05, 0x00};
  sock->async_write(ver_riplies, 2);

  uint8_t req[4];
  sock->async_read(req, 4);

  std::unique_ptr<address> addr;
  switch (req[3]) {
    case 0x01: {
      uint8_t ipv4[4];
      uint8_t port[2];
      sock->async_read(ipv4, 4);
      sock->async_read(port, 2);

      char buf[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, ipv4, buf, sizeof(buf));
      uint16_t sport = (std::uint16_t(port[0]) << 8) | port[1];
      std::cout << "received request to connect to " << buf << " : "
                << std::to_string(sport) << " \n";
      addr = std::unique_ptr<address>(new address(buf, sport));
      addr->resolve();
      // ipv4
      break;
    }
    case 0x03: {
      uint8_t len;
      sock->async_read(&len, 1);
      char buf[len + 1];
      sock->async_read(buf, len);
      buf[len] = 0;
      uint8_t port[2];
      sock->async_read(port, 2);
      uint16_t sport = (std::uint16_t(port[0]) << 8) | port[1];

      std::cout << "received request to connect to " << buf << " : "
                << std::to_string(sport) << " \n";

      addr = std::unique_ptr<address>(
          new address(buf, (std::uint16_t(port[0]) << 8) | port[1]));
      addr->resolve();
      // hostname
      break;
    }
    case 0x04:
      // ipv6
      return;
  }

  switch (req[1]) {
    case 0x01: {
      // connect
      auto rsock = std::make_shared<tcp_socket>(pool);

      uint32_t ipv4 = rsock->get_local_address().ipv4();
      uint16_t port = rsock->get_local_address().port();
      rsock->connect(*addr);
      uint8_t riply[10] = {0x05,
                           0x00,
                           0x00,
                           0x01,
                           uint8_t(ipv4 >> 24),
                           uint8_t(ipv4 >> 16),
                           uint8_t(ipv4 >> 8),
                           uint8_t(ipv4),
                           uint8_t(port >> 8),
                           uint8_t(port)};

      sock->async_write(riply, 10);

      auto piping = [rsock, sock] {
        while (1) {
          uint8_t buf[4096];
          size_t ret = rsock->async_read_some(buf, sizeof(buf));
          std::cout << "received " << std::to_string(ret) << " from remote\n";

          if (ret > 0) {
            sock->async_write(buf, ret);
          } else {
            break;
          }
        }
      };
      coroutine *co = co_create_cxx(piping, 65536);
      co_post(co);
      while (1) {
        uint8_t buf[4096];
        size_t ret = sock->async_read_some(buf, sizeof(buf));
        std::cout << "received " << std::to_string(ret) << " from local\n";

        if (ret > 0) {
          rsock->async_write(buf, ret);
        } else {
          break;
        }
      }
      break;
    }
    case 0x02:
      break;
    case 0x03:
      break;
    default:
      return;
  }
}

static void socks5_server(const ether_bridge_conf &conf,
                          epoll_event_pool &pool) {
  tcp_socket sock(pool);
  address local("0.0.0.0", 1079);
  if (local.resolve()) {
    abort();
  }
  sock.bind(local);
  sock.listen(1000);
  while (1) {
    std::shared_ptr<tcp_socket> newSock =
        std::make_shared<tcp_socket>(sock.accept());
    std::cout << "accept new socket\n";
    coroutine *co = co_create_cxx(
        std::bind(socks5_handler, std::ref(pool), newSock), 65536);
    co_post(co);
  }
}

int server(const ether_bridge_conf &conf) {
  epoll_event_pool pool;
  coroutine *co =
      co_create_cxx(std::bind(read_tap, std::ref(conf), std::ref(pool)), 65536);
  co_post(co);

  co =
      co_create_cxx(std::bind(read_udp, std::ref(conf), std::ref(pool)), 65536);
  co_post(co);

  co = co_create_cxx(std::bind(socks5_server, std::ref(conf), std::ref(pool)),
                     65536);
  co_post(co);

  pool.poll_forever();

  return 0;
}
