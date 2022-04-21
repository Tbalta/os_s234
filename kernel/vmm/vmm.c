#include "vmm.h"

#include <stdint.h>
#include "../bits/bits.h"
#include "../pmm/pmm.h"
#include "log.h"
#include "../utils/utils.h"
uint64_t get_current_cr3()
{
    uint64_t cr3;
    asm volatile("mov %%cr3, %0"
                 : "=r"(cr3));
    return cr3;
}

void load_cr3(uint64_t cr3)
{
    asm volatile("mov %0, %%cr3"
                 :
                 : "r"(cr3));
}

uint64_t get_new_plm4()
{
    return (uint64_t)pmm_alloc();
}

// uintptr_t craft_addr(uint16_t plm4, uint16_t plm3, uint16_t plm2, uint16_t plm1, uint16_t offset)
// {
//     addr_t addr = {0};
//     addr.addr.offset = offset;
//     addr.addr.plm1 = plm1;
//     addr.addr.plm2 = plm2;
//     addr.addr.plm3 = plm3;
//     addr.addr.plm4 = plm4;
//     return addr.raw;
// }

uint64_t craft_addr(uint64_t offset_l4, uint64_t offset_l3, uint64_t offset_l2, uint64_t offset_l1, uint64_t offset_l0)
{
    return offset_l0 | (offset_l1 << 12) | (offset_l2 << 21) | (offset_l3 << 30) | (offset_l4 << 39);
}

static char recursive_mapping_enabled = 0;

char is_recursive_mapping_enabled()
{
    return recursive_mapping_enabled;
}

void enable_recursive_mapping()
{
    uint64_t *cr3 = (uint64_t *)get_current_cr3();
    cr3[RECURSIVE_ENTRY] = ((uint64_t)cr3 & ~MASK(12)) | 3;
    uint64_t *recursive_address = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY * 8);
    // PAUSE();
    ASSERT(((*recursive_address) & ~MASK(12)) == (uint64_t)cr3, "Recursive mapping is not enabled expected 0x%x, got 0x%x", ((uint64_t)cr3 | 0x1), *(uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY * 8));
    LOG_OK("Recursive mapping is enabled");
    recursive_mapping_enabled = 1;
}

void disable_recursive_mapping()
{
    uint64_t *cr3 = (uint64_t *)get_current_cr3();
    cr3[RECURSIVE_ENTRY] = 0;
    recursive_mapping_enabled = 0;
}

void *find_first_empty(void *start)
{
    LOG_INFO("searching at 0x%x", start);
    addr_t address = {.raw = (uintptr_t)start};
    LOG_INFO("plm4: 0x%x, plm3: 0x%x, plm2: 0x%x, plm1: 0x%x, offset: 0x%x", address.addr.plm4, address.addr.plm3, address.addr.plm2, address.addr.plm1, address.addr.offset);
    for (size_t plm4 = address.addr.plm4; plm4 < ENTRY_PER_PAGE; plm4++, address.addr.plm4++)
    {
        if (address.addr.plm4 == RECURSIVE_ENTRY)
            continue;
        uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4 * sizeof(uint64_t));
        if (*entry == 0)
            return (void *)address.raw;
        for (size_t plm3 = address.addr.plm3; plm3 < ENTRY_PER_PAGE; address.addr.plm3++, plm3++)
        {
            uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3 * sizeof(uint64_t));
            if (*entry == 0)
                return (void *)address.raw;

            for (size_t plm2 = address.addr.plm2; plm2 < ENTRY_PER_PAGE; address.addr.plm2++, plm2++)
            {
                uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3, address.addr.plm2 * sizeof(uint64_t));
                if (*entry == 0)
                    return (void *)address.raw;

                for (size_t plm1 = address.addr.plm1; plm1 < ENTRY_PER_PAGE; address.addr.plm1++, plm1++)
                {
                    uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3, address.addr.plm2, address.addr.plm1 * sizeof(uint64_t));
                    if (*entry == 0)
                        return (void *)address.raw;
                }
                address.addr.plm1 = 0;
            }
            address.addr.plm2 = 0;
        }
        address.addr.plm3 = 0;
    }
    return NULL;
}

void *find_free_place(void *hint, size_t lenght)
{
    ASSERT(is_recursive_mapping_enabled(), "Unable to mmap recursive mapping is not enabled");
    addr_t base_address;
    lenght = ALIGN_UPPER(lenght, PAGE_SIZE);
    size_t current_length = lenght;
    if (hint == NULL)
        base_address.raw = (uintptr_t)find_first_empty((void *)craft_addr(1, 0, 0, 0, 0));
    else
        base_address.raw = (uintptr_t)find_first_empty(hint);
    addr_t current_address;
    current_address.raw = base_address.raw;
    LOG_INFO("finding free place at 0x%x of length 0x%x, starting research at 0x%x", hint, lenght, current_address.raw);

    for (; current_address.addr.plm4 < ENTRY_PER_PAGE; current_address.addr.plm4++)
    {
        if (current_address.addr.plm4 == RECURSIVE_ENTRY)
            continue;
        uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, current_address.addr.plm4 * sizeof(uint64_t));
        if (*entry == 0)
        {
            if (current_length < BYTE_PER_PLM3)
                return (void *)base_address.raw;
            current_length -= BYTE_PER_PLM3;
            continue;
        }
        for (; current_address.addr.plm3 < ENTRY_PER_PAGE; current_address.addr.plm3++)
        {
            uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, current_address.addr.plm4, current_address.addr.plm3 * sizeof(uint64_t));
            if (*entry == 0)
            {
                if (current_length < BYTE_PER_PLM2)
                    return (void *)base_address.raw;
                current_length -= BYTE_PER_PLM2;
                continue;
            }
            for (; current_address.addr.plm2 < ENTRY_PER_PAGE; current_address.addr.plm2++)
            {
                uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, current_address.addr.plm4, current_address.addr.plm3, current_address.addr.plm2 * sizeof(uint64_t));
                if (*entry == 0)
                {
                    if (current_length < BYTE_PER_PLM1)
                        return (void *)base_address.raw;
                    current_length -= BYTE_PER_PLM1;
                    continue;
                }

                for (; current_address.addr.plm1 < ENTRY_PER_PAGE; current_address.addr.plm1++)
                {
                    uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, current_address.addr.plm4, current_address.addr.plm3, current_address.addr.plm2, current_address.addr.plm1 * sizeof(uint64_t));
                    if (*entry == 1)
                        return find_free_place((void *)(current_address.raw + PAGE_SIZE), lenght); // Terminal recursion this should be optimized by the compiler (hopefully)
                    if (current_length < PAGE_SIZE)
                        return (void *)base_address.raw;
                    current_length -= PAGE_SIZE;
                }
                current_address.addr.plm1 = 0;
            }
            current_address.addr.plm2 = 0;
        }
        current_address.addr.plm3 = 0;
    }
    PANIC("e");
}

void *mmap(void *addr, size_t length, int prot)
{
    enable_recursive_mapping();
    ASSERT(is_recursive_mapping_enabled(), "Unable to mmap recursive mapping is not enabled");
    addr_t address = {.raw = (uintptr_t)find_free_place(addr, length)};
    addr_t base_address = address;

    LOG_INFO("Base address is: %x, new address is: %x", (uint64_t)addr, address.raw);
    LOG_INFO("plm4: %d, plm3: %d, plm2: %d, plm1: %d, offset: %d = %x", address.addr.plm4, address.addr.plm3, address.addr.plm2, address.addr.plm1, address.addr.offset, address.raw);

    ASSERT(address.raw != 0, "No space found");

    length = ALIGN_UPPER(length, PAGE_SIZE);
    LOG_INFO("mapping 0x%x byte at 0x%x", length, address.raw);
    for (; address.addr.plm4 < ENTRY_PER_PAGE; address.addr.plm4++)
    {
        if (address.addr.plm4 == RECURSIVE_ENTRY || address.addr.plm4 == 0 || address.addr.plm4 == 0)
            continue;
        uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4 * sizeof(uint64_t));
        if (*entry == 0)
        {
            *entry = (uint64_t)pmm_alloc();
            LOG_INFO("allocating plm4 entry at %x", entry);
        }
        *entry = *entry | PAGE_PRESENT | prot;
        ASSERT(*entry != 0, "Error while allocating page");

        for (; address.addr.plm3 < ENTRY_PER_PAGE; address.addr.plm3++)
        {
            uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3 * sizeof(uint64_t));
            if (*entry == 0)
            {
                *entry = (uint64_t)pmm_alloc();
                LOG_INFO("allocating plm3 entry at %x", entry);
            }
            *entry = *entry | PAGE_PRESENT | prot;

            ASSERT(*entry != 0, "Error while allocating page");

            for (; address.addr.plm2 < ENTRY_PER_PAGE; address.addr.plm2++)
            {
                uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3, address.addr.plm2 * sizeof(uint64_t));
                if (*entry == 0)
                {
                    *entry = (uint64_t)pmm_alloc() | PAGE_PRESENT | prot;
                    LOG_INFO("allocating plm2 entry at %x", entry);
                }
                *entry = *entry | PAGE_PRESENT | prot;

                ASSERT(*entry != 0, "Error while allocating page");
                LOG_INFO("entry: %x", *entry);
                for (size_t plm1 = address.addr.plm1; plm1 < ENTRY_PER_PAGE; address.addr.plm1++, plm1++)
                {
                    uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3, address.addr.plm2, address.addr.plm1 * sizeof(uint64_t));
                    ASSERT(*entry == 0, "Error while allocating page");
                    if (*entry == 0)
                    {
                        *entry = (uint64_t)pmm_alloc() | PAGE_PRESENT | prot;
                        LOG_INFO("mapping 0x%x byte at 0x%x with entry 0x%x: ", PAGE_SIZE, address.raw, *entry);
                    }
                    ASSERT(*entry != 0, "Error while allocating page at %x", entry);
                    // {
                    //     void *page = pmm_alloc();
                    //     *entry = (uint64_t)page | PAGE_PRESENT | prot;
                    //     LOG_INFO("page: %x", page);
                    // }
                    length -= PAGE_SIZE;
                    if (length == 0)
                    {
                        disable_recursive_mapping();
                        return (void *)base_address.raw;
                    }
                }
                address.addr.plm1 = 0;
            }
            address.addr.plm2 = 0;
        }
        address.addr.plm3 = 0;
    }
    disable_recursive_mapping();
    return (void *)base_address.raw;
}

void munmap(void *addr, size_t length)
{
    enable_recursive_mapping();
    addr_t address = {.raw = (uint64_t)addr};
    length += address.raw - ALIGN_LOWER(address.raw, PAGE_SIZE);
    address.raw = ALIGN_LOWER(address.raw, PAGE_SIZE);
    length = ALIGN_UPPER(length, PAGE_SIZE);

    LOG_INFO("unmapping 0x%x byte at 0x%x", length, address.raw);
    for (; address.addr.plm4 < ENTRY_PER_PAGE; address.addr.plm4++)
    {
        if (address.addr.plm4 == RECURSIVE_ENTRY || address.addr.plm4 == 0)
            continue;
        uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4 * sizeof(uint64_t));
        if (*entry == 0)
            return;

        for (; address.addr.plm3 < ENTRY_PER_PAGE; address.addr.plm3++)
        {
            uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3 * sizeof(uint64_t));
            if (*entry == 0)
                return;

            for (; address.addr.plm2 < ENTRY_PER_PAGE; address.addr.plm2++)
            {
                uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3, address.addr.plm2 * sizeof(uint64_t));
                if (*entry == 0)
                    return;
                for (; address.addr.plm1 < ENTRY_PER_PAGE; address.addr.plm1++)
                {
                    uint64_t *entry = (uint64_t *)craft_addr(RECURSIVE_ENTRY, address.addr.plm4, address.addr.plm3, address.addr.plm2, address.addr.plm1 * sizeof(uint64_t));
                    ASSERT(*entry != 0, "Error while unmapping page");
                    if (*entry != 0)
                    {
                        LOG_INFO("freeing entry 0x%x at logical %x", entry, address.raw);
                        pmm_free((void *)((*entry) & (~MASK(12ull))));
                        *entry = 0;
                    }
                    ASSERT(*entry == 0, "Error while unmapping page at 0x%x", address.raw);
                    length -= PAGE_SIZE;
                    LOG_INFO("remaining length: %x", length);
                    if (length == 0)
                    {
                        disable_recursive_mapping();
                        return;
                    }
                }
                address.addr.plm1 = 0;
            }
            address.addr.plm2 = 0;
        }
        address.addr.plm3 = 0;
    }
}