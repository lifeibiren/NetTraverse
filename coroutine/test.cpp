#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "coroutine.h"

#include <functional>

volatile context_t *new_stack = 0, *old_stack = 0;

void *stack;

uint64_t gettime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

uint64_t begin, end;

bool routine_dead = false;

#define REPEATE_TIME 100000

void routine(void *)
{
    float p = 1.0;
    float q = sin(p);
    printf("%f\n", q);
    for (int i = 0; i < REPEATE_TIME; i ++) {
        co_resched();
    }
    routine_dead = true;
}

void routine_bind(int a)
{
    printf("Routine Bind\n");
}

int main(void)
{
    co_init();
    coroutine_t *co = co_create(409600, (void*)routine, NULL);
    co_post(co);

    begin = gettime();
    for (int i = 0; i < REPEATE_TIME + 1; i ++) {
        co_resched();
    }
    end = gettime();
    printf("Average Cost %lf\n", ((double)end - begin) / (2 * REPEATE_TIME + 1));
    printf("Return to Main\n");

    if (!routine_dead) {
        return 1;
    }

    co = co_create_cxx([](){
        printf("CXX Lambda Coroutine\n");
        co_resched();
        printf("CXX Lambda Coroutine Exists\n");
    }, 4096);
    co_post(co);

    co_resched();
    printf("Return to Main\n");
    co_resched();

    std::function<void ()> func = std::bind(&routine_bind, 1);
    co = co_create_cxx(func, 4096);
    co_post(co);

    co_resched();
    printf("Return to Main\n");

    return 0;
}
