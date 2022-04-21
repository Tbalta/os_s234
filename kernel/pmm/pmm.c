#include "stdint.h"
#include "pmm.h"
#include "log.h"
#include "../bits/bits.h"
#include "../utils/utils.h"

static uintptr_t pmm_base = 0;
static size_t total_page_count = 0;
static struct stivale2_struct_tag_memmap *map;

void bitmap_set(void *base, size_t bit)
{
    ((uint64_t *)base)[bit / (sizeof(uint64_t) * 8)] |= (1ull << (bit % (sizeof(uint64_t) * 8)));
}

void bitmap_unset(void *base, size_t bit)
{
    ((uint64_t *)base)[bit / (sizeof(uint64_t) * 8)] &= ~(1ull << (bit % (sizeof(uint64_t) * 8)));
}


void init_bitmap(void *base, size_t size)
{
    LOG_INFO("Initializing bitmap with base %x of size %d", base, size);
    // <= can cause some issue, but w’ell hope there is enough space on this page to handle this.
    LOG_INFO("count: %d", size / (sizeof(uint64_t) * 8));
    for (size_t i = 0; i <= (size / (sizeof(uint64_t) * 8)); i++)
    {
        ((uint64_t*)base)[i] = 0;
    }
    
}

void load_memory_map(struct stivale2_struct_tag_memmap *memmap)
{

    size_t total_usable_size = 0;
    size_t writable_entry = 0;
    map = memmap;
    for (uint64_t i = 0; i < memmap->entries; i++)
    {
        struct stivale2_mmap_entry entry = memmap->memmap[i];
        LOG_INFO("Found entry at 0x%x of length 0x%x which is %s", entry.base, entry.length, (entry.type == STIVALE2_MMAP_USABLE) ? "usable" : "not really usable");
        if (entry.type == STIVALE2_MMAP_USABLE)
        {
            writable_entry++;
            total_usable_size += entry.length;
        }
    }

    total_page_count = total_usable_size / PAGE_SIZE;
    LOG_INFO("There is 0x%x byte usable for a total of %d page", total_usable_size, total_page_count);
    size_t page_in_bitmap = (total_page_count / (PAGE_SIZE * 8)) + 1;

    LOG_INFO("The bitmap will take %d page", total_page_count);

    // Searching entry big enough to contains the bitmap.
    for (uint64_t i = 0; i < memmap->entries; i++)
    {
        struct stivale2_mmap_entry entry = memmap->memmap[i];
        if (entry.type != STIVALE2_MMAP_USABLE)
            continue;
        if (entry.length * 8 < total_page_count)
            continue;
        pmm_base = entry.base;
        break;
    }
    LOG_INFO("pmm_base is located at: %x", pmm_base);
    init_bitmap((void *)pmm_base, total_page_count);
    for (size_t i = 0; i < page_in_bitmap; i++)
    {
        bitmap_set((void *)pmm_base, i);
    }
}


/**
 * @brief Convert the bitmap position to the address.
 * 
 * @param bit offset int he bitmap.
 * @param memmap pointer to the memory map structure.
 * @return size_t 
 */
uintptr_t bitmap_to_address(size_t bit)
{
    bit *= 4096;
    for (uint64_t i = 0; i < map->entries; i++)
    {
        struct stivale2_mmap_entry entry = map->memmap[i];
        if (entry.type != STIVALE2_MMAP_USABLE)
            continue;
        
        if (bit >= ALIGN_LOWER(entry.length, PAGE_SIZE))
            bit -= ALIGN_LOWER(entry.length, PAGE_SIZE);
        else
            return entry.base + bit;
    }
    PANIC("offset is out of bound");
    return -1;
}

size_t address_to_bitmap(uint64_t address)
{
    address = ALIGN_LOWER(address, PAGE_SIZE);
    // Searching entry big enough to contains the bitmap.
    size_t previous = 0;
    for (uint64_t i = 0; i < map->entries; i++)
    {
        struct stivale2_mmap_entry entry = map->memmap[i];
        if (entry.type != STIVALE2_MMAP_USABLE)
            continue;
        if (entry.base > address)
            PANIC("Unable to find 0x%x in the bitmap", address);
        if (entry.base <= address && entry.base + entry.length >= address)
            return previous + ((address - entry.base) / PAGE_SIZE);
        previous += entry.length / PAGE_SIZE;
    }
    PANIC("Unable to find 0x%x in the bitmap", address);
    // return -1;
}

size_t find_available_page(void)
{
    size_t i = 0;
    while (i < (total_page_count / (sizeof(uint64_t) * 8)) && ((uint64_t*)pmm_base)[i] == ~0ull)
    {
        i++;
    }
    // Special case in which bitmap is a multiple of 64 and there is no remaining.
    if (i * (sizeof(uint64_t) * 8) == total_page_count)
        PANIC("no page available left");
    size_t bit_total = (i * sizeof(uint64_t) * 8);
    for (int current_bit = 0; bit_total < total_page_count; bit_total++, current_bit++)
    {
        if ((((uint64_t*)pmm_base)[i] & (1ull << current_bit)) == 0)
            return bit_total;
    }
    PANIC("No page available left");    
}

void *pmm_alloc(void)
{
    size_t bit = find_available_page();
    bitmap_set((void *)pmm_base, bit);
    ASSERT(address_to_bitmap(bitmap_to_address(bit)) == bit, "error in address to bitmap");
    LOG_INFO("Allocated page at 0x%x", bitmap_to_address(bit));
    memset((void*)physical_to_stivale(bitmap_to_address(bit)), 0, PAGE_SIZE);
    return (void *)bitmap_to_address(bit);
}

void pmm_free(void *ptr)
{

    size_t bit = address_to_bitmap((uintptr_t)ptr);
    ASSERT((uintptr_t)ptr == bitmap_to_address(bit), "error in address to bitmap");
    bitmap_unset((void *)pmm_base, bit);
}
