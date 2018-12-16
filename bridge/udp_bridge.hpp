#pragma once

#include <map>

#include "socket.hpp"
#include "json.hpp"
#include "timer.hpp"

#include <time.h>
#include <functional>
#include <exception.hpp>


struct peer
{
    address addr_;
    time_t last_active_;
    int ping_;


    // todo : add timeout
    typedef std::function<void (peer *)> timeout_handler_type;
    timeout_handler_type timeout_handler_;

    peer(const address &addr) : addr_(addr)
    {}

    std::map<std::string, std::string> toJson() const
    {
        std::map<std::string, std::string> dict;
        dict["ip"] = addr_.ip();
        dict["port"] = std::to_string(addr_.port());
        dict["last_active"] = std::to_string(last_active_);
        dict["ping"] = std::to_string(ping_);

        return dict;
    }

    void keep_alive()
    {
        last_active_ = time(NULL);
    }
};

// [1 byte][1 byte][4 bytes][2 bytes][........]
// TYPE  |  ATYPE |   IPV4 |   PORT  | PAYLOAD
struct __attribute__((packed)) message_header
{
    enum message_type : uint8_t {
        PING = 0, // PING and PONG have empty message body
        PONG,
        INFO,
        DATA,
    } type;
};

struct __attribute__((packed)) address_format
{
    enum address_type : uint8_t {
        IPV4 = 0,
        IPV6 = 1,
    } type;
    uint32_t ipv4_;
    uint16_t port_;
};

class udp_bridge
{
    typedef std::map<address, peer> peer_map_type;
    peer_map_type peer_map_;

public:
    enum {udp_mtu = 1472};
    udp_bridge() {}

    void foward(udp_socket &socket, const byte_buffer &buffer, address &from)
    {
        if (peer_map_.find(from) == peer_map_.end())
        {
            peer_map_.insert(peer_map_type::value_type(from, from));
        }

        message_header *header = (message_header *)&buffer[0];
        switch (header->type)
        {
            case message_header::PING:
                break;
            case message_header::PONG:
                break;
            case message_header::INFO:
            {
                nlohmann::json j;
                for (auto it = peer_map_.begin(); it != peer_map_.end(); ++it) {
                    j.push_back(it->second.toJson());
                }
                std::string reply = j.dump();
                std::cout << "info" << reply <<std::endl;
                socket.async_send_to(from, byte_buffer(reply.c_str(), reply.size()));
                break;
            }
            case message_header::DATA:
            {
                if (buffer.size() <= sizeof(address_format) + sizeof(message_header)) {
                    return;
                }
                printf("%d %d \n", sizeof(address_format) , sizeof(message_header));
                address_format *addr_head = (address_format *)(header + 1);
                address addr;
                addr.get_sockaddr_len() = sizeof(struct sockaddr_in);
                struct sockaddr_in *sock = (struct sockaddr_in *)addr.get_sockaddr();
                sock->sin_family = AF_INET;
                sock->sin_addr.s_addr = addr_head->ipv4_;
                sock->sin_port = addr_head->port_;

                for (int i = 0; i < 16; i++) {
                    printf("%d %d \n", ((uint8_t *)(from.get_sockaddr()))[i], ((uint8_t *)(addr.get_sockaddr()))[i]);
                }
                if (addr < from || from < addr) {
                    printf("%d %d %d\n", addr.get_sockaddr_len());
                }

                auto it  = peer_map_.find(addr);
                if (it == peer_map_.end()) {
                    // no peer found
                    return;
                }

                sock = (struct sockaddr_in *)from.get_sockaddr();
                addr_head->ipv4_ = sock->sin_addr.s_addr;
                addr_head->port_ = sock->sin_port;

                socket.async_send_to(it->second.addr_, buffer);

                break;
            }
            default:
                return;
        }

        peer_map_type::iterator it = peer_map_.find(from);
        it->second.keep_alive();

//        std::string reply = it->second.toJson();
//        std::cout << reply <<std::endl;

//        socket.async_send_to(from, byte_buffer(reply.c_str(), reply.size()));

        for (auto it = peer_map_.begin(); it != peer_map_.end();) {
            if (it->second.last_active_ + 60 < time(NULL)) {
                it = peer_map_.erase(it);
            } else {
                ++it;
            }
        }
    }
};
