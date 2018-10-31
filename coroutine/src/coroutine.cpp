#include "coroutine.h"

#include <stdlib.h>
#include <map>
#include <list>

static coroutine_t *current;
static std::list<coroutine_t *> queue_;
static std::list<coroutine_t *> to_free_;

//class coroutine_table
//{
//private:
//    coroutine_t *coroutine_table = NULL;
//    static uint64_t coroutine_num = 0;
//    enum { MAX_COROUTINE = 1024};
//public:

//};

void *co_stack_create_one(void *stack, uint64_t stack_size, void *routine)
{

}

coroutine_t *co_create(uint64_t stack_size, void *routine, void *param)
{
    coroutine_t *co = (coroutine_t *)malloc(sizeof(coroutine_t));
    co->stack = malloc(stack_size);
    co->stack_size = stack_size;
    co->stack_top = co_stack_init(co->stack, co->stack_size, routine);
    co->param = param;
    co->routine = (void (*)(void *))routine;
    return co;
}

void co_launch(void)
{
    current->routine(current->param);
}

context_t *co_stack_init(void *stack, uint64_t stack_size, void *routine)
{
    // interesingly, gcc always assume (%rsp) % 16 == 8 at the beginning of a function
    uint8_t *top = (uint8_t *)stack + stack_size - 8;
    *(uint64_t *)top = (uint64_t)&co_exit; // we should place a function to handle exiting coroutine
    context_t *ctx = (context_t *)(top - sizeof(*ctx));
    ctx->RIP = (uint64_t)co_launch;
    ctx->mxcsr = mxcsr_MASK;
    ctx->x87_cw = x87_cw_mask;
    return ctx;
}

void co_init(void)
{
    if (!current)
    {
        current = (coroutine_t *)malloc(sizeof(coroutine_t));
        current->stack_size = 0;
        current->stack = NULL;
        queue_.push_back(current);
    }
//    coroutine_table = new coroutine_table[MAX_COROUTINE];

}

void co_exit(void)
{
    queue_.remove(current);
    to_free_.push_back(current);
    co_resched();
}


void co_post(coroutine_t *co)
{
//    queue_enqueue(&queue, co);
    queue_.push_back(co);
}

/*
 * this function usually return in another coroutine
 */
void co_sched(void)
{
    coroutine_t *next = *queue_.begin();
    queue_.pop_front();
    if (!next)
        return;
    coroutine_t *old = current;
    current = next;

    context_switch(&next->stack_top, &old->stack_top);

    // cleanup
    while(!to_free_.empty())
    {
        auto it = to_free_.begin();
        free((*it)->stack);
        free(*it);
        to_free_.pop_front();
    }
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

void co_yield(coroutine_t **co)
{
    *co = current;

    co_sched();
}
