file(GLOB src_file *.cpp *.cc )

set(extern_libs libyaml-cpp.a /usr/lib/openssl-1.0/libcrypto.so)
add_executable(bridge ${src_file})

target_link_libraries(bridge coroutine network compression cryptography ${extern_libs})
