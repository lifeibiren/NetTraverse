#include "compression.hpp"

#include "lz4.h"
#include <stdio.h>

byte_buffer compression::compress(const byte_buffer &buffer)
{
    int max_buffer = LZ4_compressBound(buffer.size());
    byte_buffer comp_buf(max_buffer);
    int ret = LZ4_compress_default(buffer, comp_buf, buffer.size(), comp_buf.size());
    if (ret == 0)
    {
        throw compression_failure();
    }
    comp_buf.resize(ret);
    printf("compression %d to %d, ratio %f\n", buffer.size(), comp_buf.size(), (float)buffer.size() / comp_buf.size());
    return comp_buf;
}

byte_buffer compression::decompress(const byte_buffer &buffer, std::size_t expected_max_size)
{
    byte_buffer decomp_buf(expected_max_size);
    int ret = LZ4_decompress_safe(buffer, decomp_buf, buffer.size(), decomp_buf.size());
    if (ret < 0)
    {
        throw decompression_failure();
    }
    decomp_buf.resize(ret);
    return decomp_buf;
}
