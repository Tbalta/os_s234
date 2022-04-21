#include "task.h"
#include "../log.h"
#include "../gdt.h"
#include "vmm/vmm.h"

#include "../utils/utils.h"
#include "../loader/loader.h"

static task_t task_list[10] = {0};
static size_t to_load = 0;
static size_t task_count = 0;

task_t create_task(void *module_begin, uint64_t rflags, uint64_t task_size)
{
    task_t task = {0};
    task.rsp = (uint64_t)mmap(NULL, task_size, PROT_USER | PROT_WRITE) + task_size;
    task.rip = (uint64_t)load_elf(module_begin);
    task.rflags = rflags;
    // task.cr3 = get_new_plm4();
    return task;
}

void preload_task(task_t *task)
{
    task_list[task_count++] = *task;
}

void sheduler_tick(volatile stack_frame_t *st)
{
    if (st->cs != (GDT_USER_CODE_SELECTOR | 3))
    {
        jump_into_userland(st);
        return;
    }
    task_list[to_load].regs = st->regs;
    task_list[to_load].rip = st->rip;
    // task_list[to_load].rflags = st->rflags;

    to_load = (to_load + 1) % task_count;

    st->rip = task_list[to_load].rip;
    st->rflags = task_list[to_load].rflags;
    st->regs = task_list[to_load].regs;
    // load_cr3(task_list[to_load].cr3);
}

void *get_stack_rsp()
{
    static void *task = NULL;
    if (task == NULL)
        task = (void *)(uint64_t)(mmap(NULL, 0x3000, PROT_USER | PROT_WRITE) + 0x3000);
    return task;
}

static uint64_t address_to_jump = 0;
void jump_into_module(void *module_begin)
{
    address_to_jump = (uint64_t)load_elf(module_begin);
    asm volatile("int $80");
}

void jump_into_userland(volatile stack_frame_t *st)
{
    LOG_INFO("jump_into_userland");
    st->cs = GDT_USER_CODE_SELECTOR | 3;
    st->ss = GDT_USER_DATA_SELECTOR | 3;
    st->rflags = st->rflags | (1 << 9);
    st->rsp = task_list[to_load].rsp;
    st->rip = task_list[to_load].rip;
    st->regs = task_list[to_load].regs;

    LOG_INFO("rflags is %x", st->rflags);
    LOG_INFO("rflags is %x", st->rflags | (1 << 9));
    LOG_INFO("jumping to %x", st->rip);
    LOG_INFO("rsp: %x", st->rsp);
    asm volatile(
        "mov $0x23, %%ax;"
        "mov %%ax, %%ds;"
        "mov %%ax, %%es;"
        :
        :
        : "ax");
    // PAUSE();
}
