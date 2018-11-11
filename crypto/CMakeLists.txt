cmake_minimum_required(VERSION 3.0)

Project(Cryptography)

set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

include_directories(.)
add_library(cryptography AES.cpp)

add_executable(aes_test test.cpp)
target_link_libraries(aes_test cryptography ssl crypto common)

add_test(AES_test aes_test)

target_include_directories(cryptography INTERFACE .)
