#include "AES.hpp"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <assert.h>
#include <memory>

struct EVP_auto_ptr
{
    EVP_auto_ptr()
    {
        ctx = EVP_CIPHER_CTX_new();
    }
    ~EVP_auto_ptr()
    {
        EVP_CIPHER_CTX_free(ctx);
    }
    operator EVP_CIPHER_CTX *()
    {
        return ctx;
    }
    EVP_CIPHER_CTX *ctx;
};

class cipher_buffer
{
public:
    typedef std::uint32_t length_type;
    cipher_buffer(const byte_buffer &origin) : buffer(origin)
    {
    }
    cipher_buffer(const AES_IV &iv, length_type origin_len) : buffer(origin_len + sizeof(length_type) + AES_BLOCK_SIZE * 2)
    {
        memcpy(buffer, iv.bytes, AES_BLOCK_SIZE); // record initial vector
        plaintext_len() = origin_len; // origin plain text length
    }
    length_type &plaintext_len()
    {
        return *(length_type *)(std::uint8_t *)&buffer[AES_BLOCK_SIZE];
    }

    cipher_buffer(const cipher_buffer &) = delete;
    AES_IV &iv()
    {
        return *(AES_IV *)(std::uint8_t *)buffer;
    }

    operator std::uint8_t *()
    {
        return (std::uint8_t *)&buffer[AES_BLOCK_SIZE + sizeof(length_type)];
    }

    operator byte_buffer()
    {
        return buffer;
    }

    length_type cipher_size()
    {
        return buffer.size() - AES_BLOCK_SIZE - sizeof(length_type);
    }

    void resize(length_type cipher_length)
    {
        buffer.resize(AES_BLOCK_SIZE + sizeof(length_type) + cipher_length);
    }
protected:
    byte_buffer buffer;
};

/*
 * AES won't change data's length
 */
byte_buffer AES::AES_encrypt(const byte_buffer &plain)
{
    RAND_bytes(iv_.bytes, sizeof(iv_));

    cipher_buffer cipher(iv_, plain.size());

    int len;
    int ciphertext_len;
    EVP_auto_ptr ctx;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_.bytes, iv_.bytes) != 1)
    {

    }

    if (EVP_EncryptUpdate(ctx, cipher, &len, plain, plain.size()) != 1)
    {

    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, (std::uint8_t *)cipher + len, &len) != 1)
    {

    }
    ciphertext_len += len;

    cipher.resize(ciphertext_len);

    return cipher;
}

byte_buffer AES::AES_decrypt(const byte_buffer &buffer)
{
    cipher_buffer cipher(buffer);
    iv_ = cipher.iv();

    byte_buffer plain(buffer.size());

    EVP_auto_ptr ctx;
    int len;
    int ciphertext_len;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_.bytes, iv_.bytes) != 1)
    {

    }

    if (EVP_DecryptUpdate(ctx, plain, &len, cipher, cipher.cipher_size()) != 1)
    {

    }
    ciphertext_len = len;

    if (EVP_DecryptFinal_ex(ctx, (std::uint8_t *)plain + len, &len))
    {

    }
    ciphertext_len += len;

    plain.resize(cipher.plaintext_len());
    return plain;
}
