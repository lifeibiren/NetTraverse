#include "AES.hpp"

#include <stdio.h>
#include <iostream>
const char *text = "The quick brown fox jumps over the lazy dog";

int main()
{
    byte_buffer origin_plaintext(text, strlen(text) + 1);

    AES_key key("my aes key");
    AES aes(key);
    byte_buffer &&cipher = aes.AES_encrypt(origin_plaintext);

    AES_key key2("my aes key");
    AES aes2(key2);

    byte_buffer &&plaintext = aes2.AES_decrypt(cipher);

    std::cout<<(const char *)origin_plaintext<<std::endl;
    std::cout<<(const char *)plaintext<<std::endl;

    return !(origin_plaintext == plaintext);
}
