include_directories(.)
file(GLOB protocol_src *.cc *.cpp)
add_library(protocol OBJECT ${protocol_src})
add_executable(protocol_main $<TARGET_OBJECTS:protocol>)
