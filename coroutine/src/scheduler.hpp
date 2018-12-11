#pragma once

#include <cinttypes>
#include <queue>
#include <list>

template <typename T>
class scheduler
{
public:
    virtual ~scheduler() {};
    virtual void enqueue(T t) = 0;
    virtual T dequeue() = 0;
    virtual std::size_t num() const = 0;
};

template <typename T>
class round_robin_scheduler : public scheduler<T>
{
public:
    round_robin_scheduler()
    {
    }
    virtual ~round_robin_scheduler()
    {
    }

    void enqueue(T t)
    {
        queue_.push_back(t);
    }

    T dequeue()
    {
        T t = queue_.front();
        queue_.pop_front();
        return t;
    }

    std::size_t num() const
    {
        return queue_.size();
    }

protected:
    std::list<T> queue_;
};
