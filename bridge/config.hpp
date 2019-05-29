#pragma once

#include <string>
#include <yaml-cpp/yaml.h>

#include "ether_bridge.hpp"

enum class bridge_mode {
    client,
    server,
    udp
};

int server(const ether_bridge_conf &conf);
int client(const ether_bridge_conf &conf);
// int udp_bridge_main(config &conf);

static inline void xor_mess(void *buf, std::size_t length)
{
    std::uint8_t *bytes = (std::uint8_t *)buf;
    for (std::size_t i = 0; i < length; i++)
    {
        bytes[i] ^= 0x0f;
    }
}
