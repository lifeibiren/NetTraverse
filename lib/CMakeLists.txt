include_directories(.)
file(GLOB lib_src *.cpp *.cc)
add_library(common ${lib_src})
