file(GLOB src_list *.c *.cc *.cpp asm/*.s)
include_directories(.)
add_library(compression ${src_list})
target_include_directories(compression INTERFACE .)
