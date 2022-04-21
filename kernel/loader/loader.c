#include "loader.h"

#include <stdbool.h>
#include <stddef.h>

#include "../vmm/vmm.h"
#include "../utils/utils.h"
#include "../log.h"

bool is_header_valid(elf_header_t *elf)
{
    return elf->e_ident.magic[0] == 0x7F && elf->e_ident.magic[1] == 'E' && elf->e_ident.magic[2] == 'L' && elf->e_ident.magic[3] == 'F';
}

static void *entry_point = 0;

void browse_program_header(void *elf_start, program_header_t *first_header, size_t entry_count)
{
    for (size_t i = 0; i < entry_count; i++)
    {
        program_header_t p_header = first_header[i];
        if (p_header.p_type != PT_LOAD)
            continue;
        LOG_INFO("p_vaddr: %x", p_header.p_vaddr);
        LOG_INFO("p_memsz: %x", p_header.p_memsz);
        void *data = mmap((void *)(p_header.p_vaddr), p_header.p_memsz, PROT_USER | PROT_WRITE);
        memcpy(data, (void *)(char *)(elf_start + p_header.p_offset), p_header.p_memsz);
        LOG_INFO("plouf");
        // entry_point = data;
    }
}

void *load_elf(void *elf_start)
{
    elf_header_t *elf = (elf_header_t *)elf_start;
    ASSERT(is_header_valid(elf), "header is not valid");
    browse_program_header(elf_start, (program_header_t *)((char *)elf_start + elf->e_phoff), elf->e_phnum);
    LOG_INFO("entry point: %x", elf->e_entry);
    // return entry_point;
    return (void *)elf->e_entry;
}