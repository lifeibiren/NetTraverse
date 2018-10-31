#pragma once

class event_handle
{

};

class event_handler
{
    virtual void operator()(event_handle &handle) = 0;
};

class event
{
public:
    virtual void trigger() = 0;
};

class event_pool
{
public:
    virtual void add_event(const event &handle) = 0;
    virtual void del_event(const event &handle) = 0;
    virtual void poll_one() = 0;
    virtual void poll_forever() = 0;
};
