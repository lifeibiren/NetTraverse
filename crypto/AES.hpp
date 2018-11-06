#pragma once

#include <cinttypes>
#include <string>
#include "bytebuffer.hpp"

struct AES_key
{
    std::string key;

    void to_AES_Key(std::uint8_t keybytes[32])
    {
        size_t min = key.size() < 32 ? key.size() : 32;
        for (size_t i = 0; i < min; i ++)
        {
            keybytes[i] = key[i];
        }
    }
};

struct AES_IV
{
    std::uint8_t IV[16];
};

class AES
{
public:
    AES(const AES_key &key, const AES_IV &iv) : key_(key), iv_(iv)
    {
    }
    byte_buffer AES_encrypt(const byte_buffer &buffer);
    byte_buffer AES_encrypt(const byte_buffer &buffer);
protected:
    AES_key key_;
    AES_IV iv_;
};
