#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include "gdt.h"
#include "serial.h"
#include "log.h"
#include "idt.h"
#include "interrupts/interrupts.h"
#include "pmm/pmm.h"
#include "test/test.h"
#include "vmm/vmm.h"
#include "pic/pic.h"
#include "timer/pit.h"
#include "tasking/task.h"
// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an array in .bss.
static uint8_t stack[8192];
void _start(struct stivale2_struct *stivale2_struct);

// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.

struct stivale2_struct_tag_modules module_tag =
    {
        .tag = {
            .identifier = STIVALE2_STRUCT_TAG_MODULES_ID,
            .next = 0,
        },
        .module_count = 0, // Count of loaded modules
        .modules = {{0}},    // Array of module descriptors
};

// For the paging we will want to get the memory map from the bootloader.
static struct stivale2_struct_tag_memmap memmap_tag = {
    .tag = {
        .identifier = STIVALE2_STRUCT_TAG_MEMMAP_ID,
        .next = (uint64_t)&module_tag},
    .entries = 0,
    .memmap = {{0}}};

// stivale2 offers a runtime terminal service which can be ditched at any
// time, but it provides an easy way to print out to graphical terminal,
// especially during early boot.
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        // Identification constant defined in stivale2.h and the specification.
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        // If next is 0, it marks the end of the linked list of header tags.
        .next = (uint64_t)&memmap_tag},
    // The terminal header tag possesses a flags field, leave it as 0 for now
    // as it is unused.
    .flags = 0};

// We are now going to define a framebuffer header tag.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode, if available.
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = (uint64_t)&terminal_hdr_tag},
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0};

// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = (uint64_t)_start,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want since this is a higher half kernel.
    // Bit 2, if set, tells the bootloader to enable protected memory ranges,
    // that is, to respect the ELF PHDR mandated permissions for the executable's
    // segments.
    // Bit 3, if set, enables fully virtual kernel mappings, which we want as
    // they allow the bootloader to pick whichever *physical* memory address is
    // available to load the kernel, rather than relying on us telling it where
    // to load it.
    // Bit 4 disables a deprecated feature and should always be set.
    // .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    .flags = 0,
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
    .tags = (uintptr_t)&framebuffer_hdr_tag};

// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).
void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id)
{
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;)
    {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL)
        {
            return NULL;
        }

        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id)
        {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (void *)current_tag->next;
    }
}

// Check if the ia32e mode is enabled.
char is_ia32e_enable()
{
    uint64_t efer = 0;
    asm volatile("movq $0xC0000080, %%rcx"
                 :
                 :
                 : "rcx");
    asm volatile("rdmsr");
    asm volatile("mov %%rax, %0"
                 : "=r"(efer)::"rax");
    LOG_INFO("efer: %x", efer);
    return (efer & (1 << 8)) != 0;
}

// Check if the ia32e mode is active.
char is_ia32e_active()
{
    uint32_t efer = 0;
    asm volatile("movq $0xC0000080, %rcx");
    asm volatile("rdmsr");
    asm volatile("movl %%eax, %0"
                 : "=r"(efer));

    return (efer & (1 << 9)) != 0;
}

void print_module(struct stivale2_struct_tag_modules *mod)
{
    for (size_t i = 0; i < mod->module_count; i++)
    {
        struct stivale2_module module = mod->modules[i];
        LOG_INFO("found module: %s", module.string);
        task_t task = create_task((void*)module.begin, 0, 0x4000);
        preload_task(&task);
        // jump_into_module((void*)physical_to_stivale(module.begin));
    }
}

void syscall(volatile stack_frame_t *st)
{
    LOG_INFO("got syscall from %x", st->rsp);
}

void write_msr(uint32_t msr, uint64_t value)
{
    asm volatile("wrmsr"
                 :
                 : "c"(msr), "a"(value), "d"(value >> 32));
}

// The following will be our kernel's entry point.
void _start(struct stivale2_struct *stivale2_struct)
{
    LOG_INFO("sizeof(uintptr_t) = %d", sizeof(uintptr_t));
    serial_init(11);
    gdt_init();
    idt_init();
    attach_default_exception();
    asm volatile("int $0x3");
    asm volatile("int $32");
    // Let's get the terminal structure tag from the bootloader.
    struct stivale2_struct_tag_terminal *term_str_tag;
    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);

    struct stivale2_struct_tag_memmap *memmap_str_tag;
    memmap_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);
 
    struct stivale2_struct_tag_modules *module_str_tag;
    module_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);
    ASSERT(memmap_str_tag != NULL, "memmap tag not found");
    LOG_INFO("memmap tag found at 0x%x, with %d entries", memmap_str_tag, memmap_str_tag->entries);
    load_memory_map(memmap_str_tag);
    // test_pmm();
    is_ia32e_enable();
    // PAUSE();
    enable_recursive_mapping();
    write_msr(0xC0000081, (uint64_t)&syscall);
    // test_vmm();
    load_tss(mmap(NULL, 0x2000, 0), 0x2000 - 1, get_stack_rsp());
    register_interrupt_handler(80, &jump_into_userland);
    register_interrupt_handler(0x80, &syscall);
    print_module(module_str_tag);

    // PAUSE();
    PIC_remap();
    timer_install();
    timer_add_func_tick(&sheduler_tick, 10);
    // asm volatile("int $32");
    // init_timer(1);
    // asm volatile ("cli");
    // IRQ_clear_mask(0);
    // asm volatile ("sti");
    // ffffffff8020131d
    // // Check if the tag was actually found.
    // if (term_str_tag == NULL) {
    //     // It wasn't found, just hang...
    //     for (;;) {
    //         asm ("hlt");
    //     }
    // }

    // // Let's get the address of the terminal write function.
    // void *term_write_ptr = (void *)term_str_tag->term_write;

    // // Now, let's assign this pointer to a function pointer which
    // // matches the prototype described in the stivale2 specification for
    // // the stivale2_term_write function.
    // void (*term_write)(const char *string, size_t length) = term_write_ptr;

    // // We should now be able to call the above function pointer to print out
    // // a simple "Hello World" to screen.

    // volatile int a = 5 / 0;
    // serial_send_msg("hello worldhello worldhello worldhello worldhello world");
    // LOG("salut");
    // We're done, just hang...
    for (;;)
    {
        asm("hlt");
    }
}