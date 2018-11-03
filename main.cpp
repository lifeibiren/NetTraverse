#include "epoll.hpp"
#include "socket.hpp"
#include "timer.hpp"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include <yaml-cpp/yaml.h>

void server();
int client(address server_addr);

int main(int argc, char *args[])
{
    co_init();

    setvbuf(stdout, NULL, _IONBF, 0);


    if (argc != 2)
    {
        // server in default configuration
        server();
    }
    else
    {
        YAML::Node &&node = YAML::LoadFile(args[1]);
        address server(node["ip"].as<std::string>(), node["port"].as<int>());
//        yaml_parse(args[1]);
        client(server);
    }

    return 0;
}
