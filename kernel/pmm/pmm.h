#ifndef PMM_H
#define PMM_H

#include "stivale2.h"
#include <stddef.h>

#define PAGE_SIZE 4096

void load_memory_map(struct stivale2_struct_tag_memmap *memmap);
/**
 * @brief Return the offset in the bitmap of the next avalailable page.
 *
 * @return size_t
 */
size_t find_available_page(void);
size_t address_to_bitmap(uintptr_t address);
uintptr_t bitmap_to_address(size_t bit);

void *pmm_alloc(void);
void pmm_free(void *ptr);


#define stivale_to_physical(x) (void *)(((uint64_t)x) - 0xffffffff80000000ull)
#define physical_to_stivale(x) (void *)(((uint64_t)(x)) + 0xffffffff80000000ull)

#endif