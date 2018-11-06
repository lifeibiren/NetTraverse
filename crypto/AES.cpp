#include "AES.hpp"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>


byte_buffer AES::AES_encrypt(const byte_buffer &buffer)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {

    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_, iv) != 1)
    {

    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, buffer, buffer.size()) != 1)
    {

    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext_len + len, &len))
    {

    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

}

byte_buffer AES::AES_encrypt(const byte_buffer &buffer)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {

    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
    {

    }

    if (EVP_DecryptUpdate(ctx, ciphertext, &len, buffer, buffer.size()) != 1)
    {

    }
    ciphertext_len = len;

    if (EVP_DecryptFinal_ex(ctx, ciphertext_len + len, &len))
    {

    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);


}
