#pragma once

#include <cinttypes>
#include <string>
#include "bytebuffer.hpp"


/*
 * AES encryption with random initial vector encapsulated in encrypted buffer
 */

enum {
    AES_BLOCK_SIZE = 16,
    AES_KEY_SIZE = 32
};


struct AES_key
{
    AES_key(const std::string &key)
    {
        size_t min = key.size() < AES_KEY_SIZE ? key.size() : AES_KEY_SIZE;
        size_t i;
        for (i = 0; i < min; i ++)
        {
            bytes[i] = key[i];
        }
        for (; i < AES_KEY_SIZE; i++)
        {
            bytes[i] = 0;
        }
    }
    std::uint8_t bytes[AES_KEY_SIZE];
};

struct AES_IV
{
    std::uint8_t bytes[AES_BLOCK_SIZE];
};

class AES
{
public:
    AES(const AES_key &key) : key_(key)
    {}
    byte_buffer AES_encrypt(const byte_buffer &buffer);
    byte_buffer AES_decrypt(const byte_buffer &buffer);
protected:
    AES_key key_;
    AES_IV iv_;
};
