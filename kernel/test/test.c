#include "../pmm/pmm.h"
#include "test.h"
#include "../log.h"
#include "../vmm/vmm.h"
void test_pmm()
{
    LOG_INFO("avalaible page: %d", find_available_page());
    LOG_INFO("page address: 0x%x", bitmap_to_address(find_available_page()));
    size_t page = find_available_page();
    ASSERT(address_to_bitmap(bitmap_to_address(page)) == page, "address to not match bitmap");
    void *allocated_page = pmm_alloc();
    ASSERT(address_to_bitmap((uint64_t)allocated_page) == page, "error");
    LOG_INFO("avalaible page: %d", find_available_page());
    pmm_free(allocated_page);
    LOG_INFO("avalaible page: %d", find_available_page());
}


void test_vmm()
{
    disable_recursive_mapping();
    uint64_t *data = mmap(NULL, 0x8000 * 10, 0);

    disable_recursive_mapping();
    ASSERT(data != NULL, "mmap failed");
    munmap(data, 0x8000 * 10);
    PAUSE();
    LOG_INFO("*(0x%x) = 0x%x", data, *data);
    *data = 0xdeadbeef;
    *data = 0x12345678;
    LOG_INFO("*(0x%x) = 0x%x", data, *data);
}