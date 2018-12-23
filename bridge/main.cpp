#include "epoll.hpp"
#include "socket.hpp"
#include "timer.hpp"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include <yaml-cpp/yaml.h>

#include "config.hpp"

extern void protocol_test();

int main(int argc, char *args[])
{

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc == 2) {
        std::cout<<"must specify configuration file"<<std::endl;
    } else {
        protocol_test();
    }

    co_init();

    config conf(YAML::LoadFile(args[1]));

    if (conf.mode() == bridge_mode::server)
    {
        server(conf);
    }
    else if (conf.mode() == bridge_mode::client)
    {
        client(conf);
    }
    else if (conf.mode() == bridge_mode::udp)
    {
        udp_bridge_main(conf);
    } else {

    }
    return 0;
}
