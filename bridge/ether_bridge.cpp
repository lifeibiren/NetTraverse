#include "ether_bridge.hpp"

struct ether_addr ethernet_address::broadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
ethernet_address ethernet_address::broad_cast_address(ethernet_address::broadcast);
