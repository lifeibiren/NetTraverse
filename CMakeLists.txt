cmake_minimum_required(VERSION 3.0)

Project(NetTraverse)

include(CTest)

add_compile_options(-g3)
set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 ${CMAKE_CXX_FLAGS}")

include_directories(lib)
add_subdirectory(lib)

add_subdirectory(coroutine)
add_subdirectory(network)
add_subdirectory(crypto)
add_subdirectory(compression)


set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,leak,undefined -g3")
set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address,leak,undefined -g3")

#if(CMAKE_BUILD_TYPE MATCHES DEBUG)
#    link_libraries(asan ubsan)
#endif()

include_directories($<TARGET_PROPERTY:network,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:coroutine,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:cryptography,INTERFACE_INCLUDE_DIRECTORIES>)
add_executable(main main.cpp bridge.cpp server.cpp client.cpp)
link_directories(/usr/lib/openssl-1.0)
target_link_libraries(main libyaml-cpp.a coroutine network compression cryptography libyaml-cpp.a  /usr/lib/openssl-1.0/libcrypto.so)
