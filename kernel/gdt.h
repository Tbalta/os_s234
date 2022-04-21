#ifndef GDT_H
#define GDT_H

#include <stddef.h>
#include <stdint.h>


#define GDT_ENTRIES 7
#define GDT_NULL_OFFSET 0
#define GDT_KERNEL_CODE_OFFSET 1
#define GDT_KERNEL_DATA_OFFSET 2
#define GDT_USER_CODE_OFFSET 3
#define GDT_USER_DATA_OFFSET 4
#define GDT_TSS_OFFSET 5

#define GDT_NULL_SELECTOR 0
#define GDT_KERNEL_CODE_SELECTOR 0x8
#define GDT_KERNEL_DATA_SELECTOR 0x10
#define GDT_USER_CODE_SELECTOR 0x18
#define GDT_USER_DATA_SELECTOR 0x20
#define GDT_TSS_SELECTOR 0x28

#define SEGMENT_PRESENT (1 << 7)

// Readable / Writable bit depend on the type of segment (code / data)
#define SEGMENT_WRITABLE (1 << 1)
#define SEGMENT_READABLE (1 << 1)

#define SEGMENT_EXECUTABLE (1 << 3)
#define SEGMENT_USER (3 << 5)

#define SEGMENT_SYSTEM (0 << 4)
#define SEGMENT_CODE_OR_DATA (1 << 4) // Segment user is not the right term, it defien code or data segment.

#define GDT_FLAGS_L (1 << 1)
#define GDT_FLAGS_DB (1 << 2)
#define GDT_FLAGS_GRANULARITY_4K (1 << 3)


struct gdt_segment_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_middle : 4;
    uint8_t flags : 4;
    uint8_t base_high;
} __attribute__((packed));


struct gdt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct gdt
{
    struct gdt_segment_descriptor descriptors[GDT_ENTRIES];
} __attribute__((packed));

typedef struct gdt_segment_descriptor gdt_segment_descriptor_t;
typedef struct gdt_ptr gdt_ptr_t;
typedef struct gdt gdt_t;

void gdt_init();

__attribute__((packed))
struct gdt_tss_descriptor
{
    gdt_segment_descriptor_t descriptor;
    uint32_t base_high_32;
    uint32_t reserved;
};

__attribute__((packed))
struct gdt_tss_segment{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_offset;
} __attribute__((packed));

typedef struct gdt_tss_segment gdt_tss_segment_t;


void load_tss(void *base_addr, size_t limite, void *rsp);

#endif