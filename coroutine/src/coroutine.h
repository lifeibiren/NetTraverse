#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ctx.h"
#include <stdio.h>

struct coroutine;

struct co_stack
{
    co_stack() : stack_bottom(nullptr), stack_top(nullptr), used_by(nullptr)
    {

    }
    co_stack(size_t size) : co_stack()
    {

        stack_size = size;
        stack_bottom = (uint8_t *)malloc(size);
        // interesingly, gcc always assume (%rsp) % 16 == 8 at the beginning of a function
        stack_top = stack_bottom + size - sizeof(void *);
        set_exit_guard();
    }

    // place a guard return pointer
    void set_exit_guard()
    {
        void co_exit();

        *(uint64_t *)stack_top = (uint64_t)co_exit;
    }

    ~co_stack()
    {
        free(stack_bottom);
    }

    uint8_t *stack_bottom;
    uint8_t *stack_top;
    uint64_t stack_size;
    uint64_t count_used;

    coroutine *used_by;
};


struct coroutine
{
    coroutine(bool shared_stack = false) :
        shared_stack_(shared_stack), stack(nullptr), stack_copy_(nullptr), stack_copy_size_(0)
    {
    }
    ~coroutine()
    {
        if (!shared_stack_)
        {
            delete stack;
        }
        else
        {
            if (stack)
            {
                if (stack->used_by == this)
                {
                    stack->used_by = nullptr;
                }
            }
        }
    }

    co_stack *stack;
    context_t saved_ctx;
//    void *stack;
    void *param;
    void *routine;
//    uint64_t stack_size;

    bool use_shared_stack() const
    {
        return shared_stack_;
    }
    bool in_stack() const
    {
        if (!use_shared_stack())
        {
            return true;
        }
        if (stack)
        {
            return stack->used_by == this;
        }
        else
        {
            return false;
        }
    }
    void save_stack()
    {
        stack_copy_size_ = stack->stack_top - (uint8_t *)saved_ctx.RSP;
        stack_copy_ = (uint8_t *)malloc(stack_copy_size_);
        memcpy(stack_copy_, (uint8_t *)saved_ctx.RSP, stack_copy_size_);
        printf("saved %ld\n", stack_copy_size_);
    }

    void restore_stack()
    {
        void *dst = stack->stack_top - stack_copy_size_;
        memcpy(dst, stack_copy_, stack_copy_size_);
        free(stack_copy_);
        stack->used_by = this;
        printf("restored %ld\n", stack_copy_size_);
    }

    bool shared_stack_;
    uint8_t *stack_copy_;
    size_t stack_copy_size_;
};

extern "C" void context_switch(context_t **stack_top, context_t **saved_stack_top);
extern "C" void save_context(context_t *save, void *new_stack, void (*handler)(void));
extern "C" void load_context(context_t *saved);
extern "C" void call_with_new_stack(void *new_stack, void (*handler)(void));

void co_stack_init(context_t *ctx, co_stack *stack, void *routine);

coroutine *co_create_shared(void *routine, void *param);

void co_init(void);
coroutine *co_create(uint64_t stack_size, void *routine, void *param);
void co_yield(coroutine **co); // save current context and wait to be post/dispatch
void co_post(coroutine *co); // put the coroutine to run later
void co_dispath(coroutine *co); // run the coroutine immedately
void co_destroy(coroutine *co);
void co_resched(void); // pick another coroutine to run
void co_exit(void); // destroy current coroutine
uint64_t co_num_ready(void);


template<typename FUNC>
void co_launch_cxx(void)
{
    extern coroutine *current;

    FUNC *func_ptr = (FUNC *)current->routine;
    (*func_ptr)();
    delete func_ptr;
}

template<typename FUNC>
coroutine *co_create_cxx(FUNC func, uint64_t stack_size = 0x100000)
{
    coroutine *co = new coroutine;
    co->stack = new co_stack(stack_size);
    co_stack_init(&co->saved_ctx, co->stack, (void *)&co_launch_cxx<FUNC>);
    co->routine = (void *)(new FUNC(func));
    return co;
}
