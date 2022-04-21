#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "../idt.h"
#include <stdbool.h>

struct task
{
    registers_t regs;
    uint64_t cr3;
    uint64_t rip;
    uint64_t rflags;
    uint64_t rsp;
    // bool is_loaded;
};

void jump_into_userland(stack_frame_t *st);

void *get_stack_rsp();

void jump_into_module(void *module_begin);

typedef struct task task_t;

task_t create_task(void *module_begin, uint64_t rflags, uint64_t task_size);
void preload_task(task_t *task);
void sheduler_tick(volatile stack_frame_t *st);



#endif