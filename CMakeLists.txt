cmake_minimum_required(VERSION 3.0)

Project(NetTraverse)

include(CTest)

add_compile_options(-g3 -O0)
set(CMAKE_C_FLAGS "-Wall -fstack-protector-strong ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall  -fstack-protector-strong -std=c++11 ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,leak,undefined -asan-stack=0 -g3")
set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address,leak,undefined -asan-stack=0 -g3")

include_directories(lib)
include_directories(3rdparty)
add_subdirectory(lib)

add_subdirectory(coroutine)
add_subdirectory(network)
add_subdirectory(crypto)
add_subdirectory(compression)
add_subdirectory(bridge)
add_subdirectory(utils)


#if(CMAKE_BUILD_TYPE MATCHES DEBUG)
#    link_libraries(asan ubsan)
#endif()

include_directories($<TARGET_PROPERTY:network,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:coroutine,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:cryptography,INTERFACE_INCLUDE_DIRECTORIES>)

