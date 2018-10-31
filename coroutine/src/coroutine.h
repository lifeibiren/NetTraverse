#pragma once
#include <stdint.h>
#include "ctx.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct coroutine
{
    context_t *stack_top;
    void *stack;
    void *param;
    void (*routine)(void *);
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
#endif

