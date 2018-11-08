cmake_minimum_required(VERSION 3.0)

Project(Coroutine)

# enable_language(ASM-ATT)
enable_language(ASM)
# set(ASM_DIALECT "-ATT")

# _asm_compiler(gcc)
set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

include_directories(src)
add_subdirectory(src)

add_executable(co_test test.cpp)
target_link_libraries(co_test coroutine)
