
#include "idt.h"
#include "log.h"

void divide_by_zero_handler(stack_frame_t* stack_frame)
{
    LOG_ERROR("Divide by zero exception occured at %x", stack_frame->rip);
    while (1)
        ;
}

void overflow_handler(stack_frame_t* stack_frame)
{
    LOG_ERROR("Overflow exception occured at %x", stack_frame->rip);
    while (1)
        ;
}

void double_fault_handler(stack_frame_t* stack_frame)
{
    LOG_ERROR("Double fault exception occured at %x", stack_frame->rip);
    while (1)
        ;
}

void general_protection_handler(stack_frame_t* stack_frame)
{
    LOG_ERROR("General protection exception occured at %x", stack_frame->rip);
    while (1)
        ;
}

void page_fault_handler(stack_frame_t* stack_frame)
{
    LOG_ERROR("Page fault exception occured at %x", stack_frame->rip);
    uint64_t error_code = stack_frame->error_code;
    uint64_t page_fault_address;
    asm volatile("mov %%cr2, %0"
                 : "=r"(page_fault_address));
    if (error_code & 0x1)
        LOG_ERROR("Page fault caused by a page-level protection violation");
    else
        LOG_ERROR("Page fault caused by a non-present page");

    LOG_ERROR("Page fault caused by a %s access to address %x in %s mode",
              (error_code & 0x2) ? "write" : "read",
              page_fault_address,
              (error_code & 0b100) ? "user" : "kernel");

    while (1)
        asm volatile("hlt");
}

void opcode_invalid_handler(stack_frame_t* stack_frame)
{
    LOG_ERROR("Opcode invalid: 0x%x exception occured at %x", *(uint64_t*)stack_frame->rip, stack_frame->rip);
    while (1)
        ;
}

void attach_default_exception()
{
    register_interrupt_handler(0, divide_by_zero_handler);
    register_interrupt_handler(4, overflow_handler);
    register_interrupt_handler(6, opcode_invalid_handler);
    register_interrupt_handler(8, double_fault_handler);
    register_interrupt_handler(13, general_protection_handler);
    register_interrupt_handler(14, page_fault_handler);
}