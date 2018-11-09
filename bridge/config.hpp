#pragma once

#include <string>
#include <yaml-cpp/yaml.h>

enum class bridge_mode {
    client,
    server
};

class config
{
public:
    config (const YAML::Node &node) : node_(node)
    {}

    bridge_mode mode() const
    {
        if (node_["mode"].as<std::string>() == "client")
            return bridge_mode::client;
        else if (node_["mode"].as<std::string>() == "server")
            return bridge_mode::server;
        else
            throw std::runtime_error("invalid mode");
    }

    std::string key() const
    {
        return node_["key"].as<std::string>();
    }

    std::string bind_address() const
    {
        return node_["bind_address"].as<std::string>();
    }

    std::uint16_t bind_port() const
    {
        return node_["bind_port"].as<std::uint16_t>();
    }

    std::string tap_dev() const
    {
        return node_["tap_dev"].as<std::string>();
    }

    std::string remote_address() const
    {
        return node_["remote_address"].as<std::string>();
    }

    std::uint16_t remote_port() const
    {
        return node_["remote_port"].as<std::uint16_t>();
    }

protected:
    YAML::Node node_;
};

int server(config &conf);
int client(config &conf);
