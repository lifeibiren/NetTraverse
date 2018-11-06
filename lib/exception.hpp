#pragma once

#include <exception>
#include <string>
#include <stdexcept>

struct exception : public std::exception
{
    exception()  {}
    virtual ~exception() {}
};
