#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>

void *load_elf(void *elf_start);

__attribute__((packed)) struct e_ident
{

    char magic[4];
    uint8_t ei_class;
    uint8_t ei_data;
    uint8_t ei_version;
    uint8_t ei_osabi;
    uint8_t ei_abiversion;
    uint8_t padding[7];
};

typedef struct e_ident ident_t;

__attribute__((packed)) struct elf_header
{
    ident_t e_ident;
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

typedef struct elf_header elf_header_t;

__attribute__((packed)) struct program_header
{
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_addr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

typedef struct program_header program_header_t;

__attribute__((packed)) struct section_header
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
};

typedef struct section_header section_header;

#define PT_NULL 0x00000000
#define PT_LOAD 0x00000001
#define PT_DYNAMIC 0x00000002
#define PT_INTERP 0x00000003
#define PT_NOTE 0x00000004
#define PT_SHLIB 0x00000005
#define PT_PHDR 0x00000006
#define PT_TLS 0x00000007
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6FFFFFFF
#define PT_HIPROC 0x7FFFFFFF
#endif