#include "coroutine.h"

#include <stdlib.h>
#include <list>

coroutine *current;
static std::list<coroutine *> queue_;
static std::list<coroutine *> to_free_;
enum {shared_stack_pool_size = 2};
co_stack **shared_stack_pool;
size_t shared_stack_alloc_count = 0;

void co_launch(void)
{
    ((void (*)(void *))current->routine)(current->param);
}

coroutine *co_create(uint64_t stack_size, void *routine, void *param)
{
    coroutine *co = new coroutine;
    co->stack = new co_stack(stack_size);
    co->saved_sp = co_stack_init(co->stack->stack_bottom, co->stack->stack_size, (void *)co_launch);
    co->param = param;
    co->routine = routine;
    return co;
}

coroutine *co_create_shared(void *routine, void *param)
{
    coroutine *co = new coroutine(true);
    co->stack = shared_stack_pool[shared_stack_alloc_count % shared_stack_pool_size];
    co->stack_copy_ = (uint8_t *)malloc(1024);
    co->stack_copy_size_ = 1024;
    co->saved_sp = (context_t *)(co->stack->stack_top - (co->stack_copy_ + co->stack_copy_size_ - ((uint8_t *)co_stack_init(co->stack_copy_, co->stack_copy_size_, (void *)co_launch))));
    co->param = param;
    co->routine = routine;
    return co;
}

context_t *co_stack_init(void *stack, uint64_t stack_size, void *routine)
{
    // interesingly, gcc always assume (%rsp) % 16 == 8 at the beginning of a function
    uint8_t *top = (uint8_t *)stack + stack_size - 8;
    *(uint64_t *)top = (uint64_t)&co_exit; // we should place a function to handle exiting coroutine
    context_t *ctx = (context_t *)(top - sizeof(*ctx));
    ctx->RIP = (uint64_t)routine;
    ctx->mxcsr = mxcsr_MASK;
    ctx->x87_cw = x87_cw_mask;
    return ctx;
}

void co_init(void)
{
    if (!current)
    {
        current = new coroutine;
        current->stack = new co_stack;
    }
    shared_stack_pool = new co_stack *[shared_stack_pool_size];
    for (size_t i = 0; i < shared_stack_pool_size; i ++)
    {
        shared_stack_pool[i] = new co_stack(65536);
    }
}

void co_post(coroutine *co)
{
    queue_.push_back(co);
}

void co_swap_in(coroutine *co)
{
    if (co->in_stack()) {
        return;
    }
    if (co->stack->used_by)
    {
        co->stack->used_by->save_stack();
    }
    co->restore_stack();
}

/*
 * this function usually return in another coroutine
 */
void co_sched(void)
{
    coroutine *next = *queue_.begin();
    if (!next)
        return;
    queue_.pop_front();

    coroutine *old = current;
    current = next;

    if (next->use_shared_stack()) {
        co_swap_in(next);
    }

    context_switch(&next->saved_sp, &old->saved_sp);

    // cleanup
    while(!to_free_.empty())
    {
        auto it = to_free_.begin();
        delete *it;
        to_free_.pop_front();
    }
}

void co_exit(void)
{
    queue_.remove(current);
    to_free_.push_back(current);
    co_sched();
}

void co_resched(void)
{
    queue_.push_back(current);

    co_sched();
}

uint64_t co_num_ready(void)
{
    return queue_.size();
}

void co_yield(coroutine **co)
{
    *co = current;

    co_sched();
}
