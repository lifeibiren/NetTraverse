#pragma once

#include "bytebuffer.hpp"

#include "exception.hpp"

struct compression_failure : exception {};
struct decompression_failure : exception {};

class compression
{
public:
    static byte_buffer compress(const byte_buffer &buffer);
    static byte_buffer decompress(const byte_buffer &buffer, std::size_t expected_max_size);
};
