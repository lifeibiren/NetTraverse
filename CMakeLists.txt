cmake_minimum_required(VERSION 3.0)

Project(NetTraverse)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

add_subdirectory(coroutine)
add_subdirectory(network)
add_subdirectory(crypto)

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,leak,undefined -g3")
set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address,leak,undefined -g3")

#if(CMAKE_BUILD_TYPE MATCHES DEBUG)
#    link_libraries(asan ubsan)
#endif()

include_directories($<TARGET_PROPERTY:network,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:coroutine,INTERFACE_INCLUDE_DIRECTORIES>)
add_executable(main main.cpp bridge.cpp server.cpp client.cpp)
target_link_libraries(main libyaml-cpp.a coroutine network libyaml-cpp.a)
