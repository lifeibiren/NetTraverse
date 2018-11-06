cmake_minimum_required(VERSION 3.0)

Project(Crypto)

set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

include_directories(.)
add_library(crypto AES.cpp)

add_executable(crypto_test test.cpp)
target_link_libraries(crypto_test crypto)
