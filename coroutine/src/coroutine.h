#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "ctx.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct coroutine
{
    context_t *stack_top;
    void *stack;
    void *param;
    void *routine;
    uint64_t stack_size;
    uint64_t id;
} coroutine_t;

void context_switch(context_t **stack_top, context_t **saved_stack_top);
context_t *co_stack_init(void *stack, uint64_t stack_size, void *routine);


void co_init(void);
coroutine_t *co_create(uint64_t stack_size, void *routine, void *param);
void co_yield(coroutine_t **co); // save current context and wait to be post/dispatch
void co_post(coroutine_t *co); // put the coroutine to run later
void co_dispath(coroutine_t *co); // run the coroutine immedately
void co_destroy(coroutine_t *co);
void co_resched(void); // pick another coroutine to run
void co_exit(void); // destroy current coroutine
uint64_t co_num_ready(void);

#ifdef __cplusplus
}

template<typename FUNC>
void co_launch_cxx(void)
{
    extern coroutine_t *current;

    FUNC *func_ptr = (FUNC *)current->routine;
    (*func_ptr)();
    delete func_ptr;
}

template<typename FUNC>
coroutine_t *co_create_cxx(FUNC func, uint64_t stack_size = 0x100000)
{
    coroutine_t *co = (coroutine_t *)malloc(sizeof(coroutine_t));
    co->stack = malloc(stack_size);
    co->stack_size = stack_size;
    co->stack_top = co_stack_init(co->stack, co->stack_size, (void *)&co_launch_cxx<FUNC>);
    co->routine = (void *)(new FUNC(func));
    return co;
}

#endif
