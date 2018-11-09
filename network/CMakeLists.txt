cmake_minimum_required(VERSION 3.0)

Project(network)

include_directories(.)
include_directories($<TARGET_PROPERTY:coroutine,INTERFACE_INCLUDE_DIRECTORIES>)

add_subdirectory(epoll)


file(GLOB network_src *.cc *.cpp)
add_library(network $<TARGET_OBJECTS:network_epoll> ${network_src})
target_include_directories(network INTERFACE epoll .)
