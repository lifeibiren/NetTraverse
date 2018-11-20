#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ctx.h"
#include <stdio.h>

extern "C" {
void context_switch(context_t **stack_top, context_t **saved_stack_top);
void save_context(context_t *save, void *new_stack, void (*handler)(void));
void load_context(context_t *saved);
void call_with_new_stack(void *new_stack, void (*handler)(void));
void co_memcpy(void *dst, void *src, size_t len);
}


void co_exit();
void co_launch(void);


struct coroutine;

// interesingly, gcc always assume (%rsp) % 16 == 8 at the beginning of a function
struct co_stack
{
    co_stack() : stack_bottom(nullptr), stack_top(nullptr), used_by(nullptr), shared(false)
    {

    }
    co_stack(size_t size, bool share = false) : co_stack()
    {
        shared = share;
        stack_size = size;
        stack_bottom = (uint8_t *)malloc(size);
        stack_top = stack_bottom + size;
    }

    void set_exit_guard()
    {
        *(uint64_t *)(stack_top - sizeof(uint64_t)) = (uint64_t)co_exit;
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
    bool shared;
};


struct coroutine
{
    coroutine(void *routine, void *param = nullptr, bool shared_stack = false) :
        routine(routine), param(param), stack(nullptr),
        shared_stack_(shared_stack), stack_copy_(nullptr), stack_copy_size_(0)
    {
        bzero(&saved_ctx, sizeof(saved_ctx));

        saved_ctx.RIP = (uint64_t)co_launch;
        saved_ctx.mxcsr = mxcsr_MASK;
        saved_ctx.x87_cw = x87_cw_mask;

        if (shared_stack)
        {
            // return guard
            stack_copy_ = (uint8_t *)malloc(8);
            stack_copy_size_ = 8;
            *(uint64_t *)(stack_copy_) = (uint64_t)co_exit;
        }
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
            if (stack_copy_)
            {
                delete stack_copy_;
            }
        }
    }


    void set_stack(co_stack *new_stack)
    {
        if (shared_stack_ && !new_stack->shared)
        {
            abort();
        }
        if (!shared_stack_)
        {
            new_stack->set_exit_guard();
        }
        stack = new_stack;
        saved_ctx.RSP = (uint64_t)new_stack->stack_top - sizeof(uint64_t);
    }

    context_t saved_ctx;
    void *routine;
    void *param;
    co_stack *stack;

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
#if 0
        memcpy(stack_copy_, (uint8_t *)saved_ctx.RSP, stack_copy_size_);
#else
        co_memcpy(stack_copy_, (uint8_t *)saved_ctx.RSP, stack_copy_size_);
#endif
//        printf("saved %ld\n", stack_copy_size_);
    }

    void restore_stack()
    {
        void *dst = stack->stack_top - stack_copy_size_;
#if 0
        memcpy(dst, stack_copy_, stack_copy_size_);
#else
        co_memcpy(dst, stack_copy_, stack_copy_size_);
#endif
        free(stack_copy_);
        stack_copy_ = nullptr;
        stack->used_by = this;
//        printf("restored %ld\n", stack_copy_size_);
    }

    bool shared_stack_;
    uint8_t *stack_copy_;
    size_t stack_copy_size_;
};

void co_stack_init(context_t *ctx, co_stack *stack, void *routine);


void co_init(void);
coroutine *co_create(uint64_t stack_size, void *routine, void *param);
coroutine *co_create_shared(void *routine, void *param);
void co_yield(coroutine **co); // save current context and wait to be post/dispatch
void co_post(coroutine *co); // put the coroutine to run later
void co_dispath(coroutine *co); // run the coroutine immedately
void co_destroy(coroutine *co);
void co_resched(void); // pick another coroutine to run
void co_exit(void); // destroy current coroutine
co_stack *co_alloc_shared_stack(void);
uint64_t co_num_ready(void);


template<typename FUNC>
void co_launch_cxx(void *param)
{
    FUNC *func_ptr = (FUNC *)param;
    (*func_ptr)();
    delete func_ptr;
}

template<typename FUNC>
coroutine *co_create_cxx(FUNC func, uint64_t stack_size = 0x100000)
{
    coroutine *co = new coroutine((void *)&co_launch_cxx<FUNC>, (void *)new FUNC(func));
    co_stack *stack = new co_stack(stack_size);
    co->set_stack(stack);
    return co;
}

template<typename FUNC>
coroutine *co_create_cxx_shared(FUNC func)
{
    coroutine *co = new coroutine((void *)&co_launch_cxx<FUNC>, (void *)new FUNC(func), true);
    co->set_stack(co_alloc_shared_stack());
    return co;
}
