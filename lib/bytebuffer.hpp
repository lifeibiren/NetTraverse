#pragma once

#include <cinttypes>
#include <cstring>
#include <exception.hpp>

struct buffer_overflow : exception {};

class byte_buffer
{
public:
    byte_buffer() : ptr_(nullptr), size_(0)
    {}
    byte_buffer(std::size_t size) : ptr_(new std::uint8_t[size]), size_(size)
    {}
    byte_buffer(const void *ptr, std::size_t len) : byte_buffer(len)
    {
        memcpy(ptr_, ptr, len);
    }
    byte_buffer(const byte_buffer &buffer) : byte_buffer(buffer.ptr_, buffer.size_)
    {
    }
    byte_buffer(byte_buffer &&buffer)
    {
        ptr_ = buffer.ptr_;
        size_ = buffer.size_;

        buffer.ptr_ = nullptr;
        buffer.size_ = 0;
    }
    virtual ~byte_buffer()
    {
        delete[] ptr_;
    }

    operator void *()
    {
        return ptr_;
    }
    operator std::uint8_t *()
    {
        return ptr_;
    }
    operator char *()
    {
        return (char *)ptr_;
    }
    operator const void *() const
    {
        return ptr_;
    }
    operator const std::uint8_t *() const
    {
        return ptr_;
    }
    operator const char *() const
    {
        return (const char *)ptr_;
    }
    std::size_t size() const
    {
        return size_;
    }
    bool operator==(const byte_buffer &rms) const
    {
        if (size_ != rms.size_)
            return false;
        return memcmp(ptr_, rms.ptr_, size_) == 0;
    }
    std::uint8_t &operator[](std::size_t index)
    {
        if (index > size_)
        {
            std::uint8_t *new_ptr = new std::uint8_t[size_ * 2];
            bcopy(ptr_, new_ptr, size_);
            delete []ptr_;
            ptr_ = new_ptr;
        }
        return ptr_[index];
    }
    const std::uint8_t &operator[](std::size_t index) const
    {
        if (index > size_)
        {
            throw buffer_overflow();
        }
        return ptr_[index];
    }
    std::size_t resize(std::size_t size)
    {
        if (size > size_)
        {
            std::uint8_t *new_ptr = new std::uint8_t[size_ * 2];
            bcopy(ptr_, new_ptr, size > size_ ? size_ : size);
            delete []ptr_;
            ptr_ = new_ptr;
            size_ = size;
        }
        else
        {
            size_ = size;
        }
        return size_;
    }
    std::size_t fill(void *buf, std::size_t size, std::size_t offset = 0) const
    {
        std::size_t to_copy = size_ - offset;
        to_copy = to_copy > size ? size : to_copy;
        bcopy(buf, ptr_ + offset, to_copy);
        return to_copy;
    }
protected:
    std::uint8_t *ptr_;
    std::size_t size_;
};
