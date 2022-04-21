#ifndef VMM_H
#define VMM_H
#include <stddef.h>
#include <stdint.h>

#define PROT_WRITE (1 << 1)
#define PROT_USER (1 << 2)
#define RECURSIVE_ENTRY 10
void *mmap(void *addr, size_t length, int prot);
void munmap(void *addr, size_t length);
void enable_recursive_mapping();
uint64_t get_new_plm4();
void load_cr3(uint64_t cr3);

#define ENTRY_PER_PAGE 512ull
#define PAGE_PRESENT 0X1
#define BYTE_PER_PLM1 (PAGE_SIZE * ENTRY_PER_PAGE)
#define BYTE_PER_PLM2 (ENTRY_PER_PAGE * BYTE_PER_PLM1)
#define BYTE_PER_PLM3 (ENTRY_PER_PAGE * BYTE_PER_PLM2)
#define BYTE_PER_PLM4 (ENTRY_PER_PAGE * BYTE_PER_PLM3)

__attribute__((packed)) struct address_offset_64
{
    uint16_t offset : 12;
    uint16_t plm1 : 9;
    uint16_t plm2 : 9;
    uint16_t plm3 : 9;
    uint16_t plm4 : 9;
    uint16_t padding;
} __attribute__((packed));

typedef struct address_offset_64 address_offset_64_t;

__attribute__((packed)) union addr
{
    address_offset_64_t addr;
    uint64_t raw;
} __attribute__((packed));

typedef union addr addr_t;

// int test = sizeof(uintptr_t);

// int trac = sizeof(address_offset_64_t);

void disable_recursive_mapping();

#endif