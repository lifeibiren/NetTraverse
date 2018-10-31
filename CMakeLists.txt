cmake_minimum_required(VERSION 3.0)

Project(NetTraverse)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

add_subdirectory(coroutine)
add_subdirectory(network)


message(STATUS $<target_property:network,include_directories>)
include_directories($<TARGET_PROPERTY:network,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:coroutine,INTERFACE_INCLUDE_DIRECTORIES>)
add_executable(main main.cpp)
target_link_libraries(main coroutine network)
