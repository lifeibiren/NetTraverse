#include "coroutine.h"

#include <stdlib.h>
#include <list>

coroutine *current;
static std::list<coroutine *> queue_;
enum {shared_stack_pool_size = 1};
co_stack **shared_stack_pool;
size_t shared_stack_alloc_count = 0;

#define STACK_SIZE (4096 + 8)
uint8_t stack[STACK_SIZE];

void co_launch(void)
{
    ((void (*)(void *))current->routine)(current->param);
}

co_stack *co_alloc_shared_stack(void)
{
    return shared_stack_pool[shared_stack_alloc_count++ % shared_stack_pool_size];
}

coroutine *co_create(uint64_t stack_size, void *routine, void *param)
{
    coroutine *co = new coroutine((void *)routine, param);
    co_stack *stack = new co_stack(stack_size);
    co->set_stack(stack);
    return co;
}

coroutine *co_create_shared(void *routine, void *param)
{
    coroutine *co = new coroutine(routine, param, true);
    co->set_stack(co_alloc_shared_stack());
    return co;
}

void co_stack_init(context_t *ctx, co_stack *stack, void *routine)
{
    ctx->RSP = (uint64_t)stack->stack_top;
    ctx->RIP = (uint64_t)routine;
    ctx->mxcsr = mxcsr_MASK;
    ctx->x87_cw = x87_cw_mask;
}

void co_init(void)
{
    if (!current)
    {
        current = new coroutine(nullptr);
        current->stack = new co_stack;
        current->stack->used_by = current;
    }

    shared_stack_pool = new co_stack *[shared_stack_pool_size];
    for (size_t i = 0; i < shared_stack_pool_size; i ++)
    {
        shared_stack_pool[i] = new co_stack(1024 * 1024, true);
    }
}

void co_post(coroutine *co)
{
    queue_.push_back(co);
}

void co_swap_in(coroutine *co)
{
    if (co->in_stack())
    {
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

    current = next;

    if (next->use_shared_stack()) {
        co_swap_in(next);
    }

    load_context(&current->saved_ctx);
}


void _co_exit(void)
{
    queue_.remove(current);
    delete current;
    co_sched();
}

void co_exit(void)
{
    call_with_new_stack(&stack[STACK_SIZE], &_co_exit);
}

void _co_resched(void)
{
    queue_.push_back(current);

    co_sched();
}

void co_resched(void)
{
    save_context(&current->saved_ctx, &stack[STACK_SIZE], &_co_resched);
}

void _co_yield(void)
{
    co_sched();
}

void co_yield(coroutine **co)
{
    *co = current;

    save_context(&current->saved_ctx, &stack[STACK_SIZE], &_co_yield);
}


uint64_t co_num_ready(void)
{
    return queue_.size();
}

