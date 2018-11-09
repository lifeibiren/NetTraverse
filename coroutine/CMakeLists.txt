cmake_minimum_required(VERSION 3.0)

Project(Coroutine)

enable_language(ASM)

set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

include_directories(src)
add_subdirectory(src)

add_executable(co_test test.cpp)
target_link_libraries(co_test coroutine)
add_test(co_test co_test)
