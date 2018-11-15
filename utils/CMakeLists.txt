include_directories(.)

file(GLOB utils_src *.cc *.cpp)
add_library(utils ${utils_src})
target_include_directories(utils INTERFACE .)
