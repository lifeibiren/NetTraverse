#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "coroutine.h"

volatile context_t *new_stack = 0, *old_stack = 0;

void *stack;

uint64_t gettime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

uint64_t begin, end;

void routine(void)
{
    for (int i = 0; i < 10; i ++) {
     float p = 1.0;
     float q = sin(p);
     printf("%f\n", q);
        printf("Coroutine 1\n");
        co_resched();
    }
}


int main(void)
{
    co_init();
    coroutine_t *co = co_create(409600, (void*)routine, NULL);
    co_post(co);

    begin = gettime();

    double t = 1.0;
    double f = 2.0;
    double x = t + f;
    
    for (int i = 0; i < 10; i ++) {
        co_resched();
        printf("Return to Main\n");
    }
    end = gettime();
    printf("%lf\n", x);


    printf("Cost %lu nsecs\n", end - begin);
    return 0;
}
