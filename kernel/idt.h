#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_FLAGS_PRESENT (1 << 7)
#define IDT_FLAGS_INTERRUPT_GATE (0b1110)
#define IDT_FLAGS_RING_3 (0b11 << 5)
__attribute__((packed)) struct registers
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15;
};

typedef struct registers registers_t;


struct idt_entry
{
    uint16_t offset_0_15;
    uint16_t segment_selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset_16_31;
    uint32_t offset_32_63;
    uint32_t reserved;
} __attribute__((packed));

struct idtr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

__attribute__((packed))
struct stack_frame
{
    registers_t regs;
    uint64_t int_no;
    uint64_t error_code;


    uint64_t rip;
    uint16_t cs;
    uint16_t align3;
    uint32_t align4;
    uint64_t rflags;
    uint64_t rsp;
    uint16_t ss;
} __attribute__((packed));

typedef volatile struct stack_frame stack_frame_t;
typedef void (*interrupt_handler_t)(stack_frame_t *stack_frame);


typedef struct idtr idtr_t;
typedef struct idt_entry idt_entry_t;

void idt_init();

void exception_handler(stack_frame_t stack_frame);
void irq_handler(volatile stack_frame_t stack_frame);
void register_interrupt_handler(uint8_t interrupt_number, interrupt_handler_t handler);

extern void irq_0();
extern void irq_1();
extern void irq_2();

#endif