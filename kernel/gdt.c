#include "gdt.h"
#include "log.h"
static gdt_t gdt = {0};


gdt_segment_descriptor_t create_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdt_segment_descriptor_t descriptor;

    descriptor.base_low = (base & 0xFFFF);
    descriptor.base_middle = (base >> 16) & 0xFF;
    descriptor.base_high = (base >> 24) & 0xFF;

    descriptor.limit_low = (limit & 0xFFFF);
    descriptor.flags = ((limit >> 16) & 0x0F);

    descriptor.flags = flags;
    descriptor.access = access;

    return descriptor;
}

void gdt_init(){

    gdt.descriptors[GDT_NULL_OFFSET] = create_descriptor(0, 0, 0, 0);
    gdt.descriptors[GDT_KERNEL_CODE_OFFSET] = create_descriptor(0, 0xFFFFF, SEGMENT_PRESENT | SEGMENT_CODE_OR_DATA | SEGMENT_EXECUTABLE | SEGMENT_READABLE, GDT_FLAGS_L | GDT_FLAGS_GRANULARITY_4K);
    LOG_INFO("kernel_code_OFFSET {.access: 0x%x, .flags: 0x%x}", gdt.descriptors[GDT_KERNEL_CODE_OFFSET].access, gdt.descriptors[GDT_KERNEL_CODE_OFFSET].flags);

    gdt.descriptors[GDT_KERNEL_DATA_OFFSET] = create_descriptor(0, 0xFFFFF, SEGMENT_PRESENT | SEGMENT_CODE_OR_DATA | SEGMENT_WRITABLE, GDT_FLAGS_DB | GDT_FLAGS_GRANULARITY_4K);
    LOG_INFO("kernel_data_OFFSET {.access: 0x%x, .flags: 0x%x}", gdt.descriptors[GDT_KERNEL_DATA_OFFSET].access, gdt.descriptors[GDT_KERNEL_DATA_OFFSET].flags);

    gdt.descriptors[GDT_USER_CODE_OFFSET] = create_descriptor(0, 0xFFFFF, SEGMENT_PRESENT | SEGMENT_CODE_OR_DATA | SEGMENT_EXECUTABLE | SEGMENT_USER | SEGMENT_READABLE, GDT_FLAGS_L | GDT_FLAGS_GRANULARITY_4K);
    LOG_INFO("user_code_OFFSET {.access: 0x%x, .flags: 0x%x}", gdt.descriptors[GDT_USER_CODE_OFFSET].access, gdt.descriptors[GDT_USER_CODE_OFFSET].flags);

    gdt.descriptors[GDT_USER_DATA_OFFSET] = create_descriptor(0, 0xFFFFF, SEGMENT_PRESENT | SEGMENT_CODE_OR_DATA | SEGMENT_WRITABLE | SEGMENT_USER, GDT_FLAGS_DB | GDT_FLAGS_GRANULARITY_4K);
    LOG_INFO("user_data_OFFSET {.access: 0x%x, .flags: 0x%x}", gdt.descriptors[GDT_USER_DATA_OFFSET].access, gdt.descriptors[GDT_USER_DATA_OFFSET].flags);
    //TODO: Load the TSS

    gdt_ptr_t gdt_ptr;
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;
    extern void load_gdt(gdt_ptr_t* gdt_ptr);
    load_gdt(&gdt_ptr);
    LOG_OK("gdt is loaded with base %x, gdt limit is %d", gdt_ptr.base, gdt_ptr.limit);
}

void load_tss(void *base_addr, size_t limite, void *rsp)
{
    LOG_INFO("rsp: %x",(uint64_t)rsp);
    gdt_tss_segment_t *segment = (gdt_tss_segment_t *)base_addr;
    asm volatile ("mov %%rsp, %0" : "=r" (segment->rsp0));
    segment->rsp2 = (uint64_t)rsp;
    gdt.descriptors[GDT_TSS_OFFSET] = create_descriptor((uint32_t)(uint64_t)base_addr, limite, SEGMENT_PRESENT | SEGMENT_SYSTEM | SEGMENT_EXECUTABLE | 0x9, GDT_FLAGS_GRANULARITY_4K);
    *((uint64_t*)(&gdt.descriptors[GDT_TSS_OFFSET + 1])) = ((uint32_t)((uint64_t)base_addr>>32));
    LOG_INFO("tss_OFFSET {.access: 0x%x, .flags: 0x%x}", gdt.descriptors[GDT_TSS_OFFSET].access, gdt.descriptors[GDT_TSS_OFFSET].flags);
    asm volatile ("ltr %%ax"::"a"(GDT_TSS_SELECTOR));
}