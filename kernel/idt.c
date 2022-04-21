#include "idt.h"

#include "log.h"

// #define DEBUG_IDT

__attribute__((aligned(0x10))) static idt_entry_t idt[256] = {0}; // Create an array of IDT entries; aligned for performance

static interrupt_handler_t interrupt_handlers[256] = {0}; // Create an array of pointers to interrupt handlers

idt_entry_t create_entry(uint64_t offset, uint16_t selector, uint8_t attribute)
{
    idt_entry_t entry;
    entry.flags = attribute;
    entry.ist = 0;
    entry.segment_selector = selector;
    entry.offset_0_15 = offset & 0xFFFF;
    entry.offset_16_31 = (offset >> 16) & 0xFFFF;
    entry.offset_32_63 = offset >> 32;
    entry.reserved = 0;

    return entry;
}

void idt_init()
{
    extern void *isr_stub_table[];
    for (int i = 0; i < 32; i++)
    {
        idt[i] = create_entry((uint64_t)isr_stub_table[i], 0x08, IDT_FLAGS_PRESENT | IDT_FLAGS_INTERRUPT_GATE);
#ifdef DEBUG_IDT
        LOG_INFO("isr[%d] = %x", i, isr_stub_table[i]);
        LOG_INFO("flags %x", idt[i].flags);
        LOG_INFO("segment_selector %x", idt[i].segment_selector);
        LOG_INFO("offset: %x", (uint64_t)idt[i].offset_0_15 | (uint64_t)((uint64_t)idt[i].offset_16_31 << 16ull) | (uint64_t)((uint64_t)idt[i].offset_32_63 << 32ull));
#endif
    }

    idt[32] = create_entry((uint64_t)irq_0, 0x08, IDT_FLAGS_PRESENT | IDT_FLAGS_INTERRUPT_GATE);
    idt[80] = create_entry((uint64_t)irq_1, 0x08, IDT_FLAGS_PRESENT | IDT_FLAGS_INTERRUPT_GATE);
    idt[0x80] = create_entry((uint64_t)irq_2, 0x08, IDT_FLAGS_PRESENT | IDT_FLAGS_INTERRUPT_GATE | IDT_FLAGS_RING_3);
    idtr_t idtr;
    idtr.base = (uint64_t)idt;
    idtr.limit = sizeof(idt) - 1;
    __asm__ volatile("lidt %0"
                     :
                     : "m"(idtr)); // load the new IDT
    LOG_OK("idt is loaded at {.base = 0x%x, limit = %d}", idtr.base, idtr.limit);
    __asm__ volatile("sti"); // set the interrupt flag
    LOG_INFO("enabling interrupt", 0);
}

void register_interrupt_handler(uint8_t interrupt_number, interrupt_handler_t handler)
{
    interrupt_handlers[interrupt_number] = handler;
}

void exception_handler(stack_frame_t stack_frame)
{
    LOG_INFO("{INT_NO: %x,ERR_CODE: %x, RIP: %x, RSP: %x, cs:%x}", stack_frame.int_no, stack_frame.error_code, stack_frame.rip, stack_frame.rsp, stack_frame.cs);
    if (interrupt_handlers[stack_frame.int_no] == NULL)
    {
        LOG_ERROR("Interrupt %xh has no handlers bound !", stack_frame.int_no);
    }
    else
    {
        interrupt_handlers[stack_frame.int_no](&stack_frame);
    }
}

void irq_handler(stack_frame_t stack_frame)
{
    // LOG_INFO("{INT_NO: %x,ERR_CODE: %x, RIP: %x}", stack_frame.int_no, stack_frame.error_code, stack_frame.rip);
    if (interrupt_handlers[stack_frame.int_no] == NULL)
    {
        LOG_ERROR("Interrupt %xh has no handlers bound !", stack_frame.int_no);
    }
    else
    {
        interrupt_handlers[stack_frame.int_no](&stack_frame);
    }
}