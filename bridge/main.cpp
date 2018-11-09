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


int main(int argc, char *args[])
{
    co_init();

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc != 2) {
        std::cout<<"must specify configuration file"<<std::endl;
    }

    config conf(YAML::LoadFile(args[1]));

    if (conf.mode() == bridge_mode::server)
    {
        server(conf);
    }
    else
    {
        client(conf);
    }

    return 0;
}