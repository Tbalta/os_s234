#ifndef PIT_H
#define PIT_H

#include "../idt.h"

#include <stddef.h>

void timer_install();
void init_timer(int hz);

struct timer_func
{
    void (*func)(stack_frame_t *stack_frame);
    size_t tick_to_wait;
};


typedef struct timer_func timer_func_t;

void timer_add_func_tick(void (*func)(stack_frame_t *stack_frame), size_t tick);
void timer_add_func(void (*func)(stack_frame_t *stack_frame), int hz);

#endif