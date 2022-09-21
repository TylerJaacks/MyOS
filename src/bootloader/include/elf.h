#ifndef BOOTLOADER_ELF_H
#define BOOTLOADER_ELF_H 1

#include <efi.h>
#include <efilib.h>

#include "debug.h"

#define EI_NIDENT        16

#define EI_MAG0          0
#define EI_MAG1          0x1
#define EI_MAG2          0x2
#define EI_MAG3          0x3
#define EI_CLASS         0x4
#define EI_DATA          0x5
#define EI_VERSION       0x6
#define EI_OSABI         0x7
#define EI_ABIVERSION    0x8

#define PT_NULL            0
#define PT_LOAD            1
#define PT_DYNAMIC         2
#define PT_INTERP          3
#define PT_NOTE            4
#define PT_SHLIB           5
#define PT_PHDR            6
#define PT_TLS             7

typedef enum e_file_class {
	ELF_FILE_CLASS_NONE = 0,
	ELF_FILE_CLASS_32 = 1,
	ELF_FILE_CLASS_64 = 2,
} Elf_File_Class;

typedef struct s_elf32_hdr {
	unsigned char	e_ident[EI_NIDENT];
	UINT16 e_type;
	UINT16 e_machine;
	UINT32 e_version;
	UINT32 e_entry;
	UINT32 e_phoff;
	UINT32 e_shoff;
	UINT32 e_flags;
	UINT16 e_ehsize;
	UINT16 e_phentsize;
	UINT16 e_phnum;
	UINT16 e_shentsize;
	UINT16 e_shnum;
	UINT16 e_shstrndx;
} Elf32_Ehdr;

typedef struct s_elf64_hdr {
	unsigned char	e_ident[EI_NIDENT];
	UINT16 e_type;
	UINT16 e_machine;
	UINT32 e_version;
	UINT64 e_entry;
	UINT64 e_phoff;
	UINT64 e_shoff;
	UINT32 e_flags;
	UINT16 e_ehsize;
	UINT16 e_phentsize;
	UINT16 e_phnum;
	UINT16 e_shentsize;
	UINT16 e_shnum;
	UINT16 e_shstrndx;
} Elf64_Ehdr;

typedef struct s_elf32_phdr {
	UINT32 p_type;
	UINT32 p_offset;
	UINT32 p_vaddr;
	UINT32 p_paddr;
	UINT32 p_filesz;
	UINT32 p_memsz;
	UINT32 p_flags;
	UINT32 p_align;
} Elf32_Phdr;

typedef struct s_elf64_phdr {
	UINT32 p_type;
	UINT32 p_flags;
	UINT64 p_offset;
	UINT64 p_vaddr;
	UINT64 p_paddr;
	UINT64 p_filesz;
	UINT64 p_memsz;
	UINT64 p_align;
} Elf64_Phdr;

EFI_STATUS ReadElfFile(
    IN EFI_FILE* const KernelImageFile,
    // FIXEME: IN Elf_File_Class const FileClass, 
	IN UINT8 const FileClass,
    OUT VOID** KernelHeaderBuffer, 
    OUT VOID** KernelProgramHeadersBuffer);

EFI_STATUS ReadElfIdentity(IN EFI_FILE* const KernelImageFile, OUT UINT8** ElfIdentityBuffer);

EFI_STATUS ValidateElfIdentity(IN UINT8* const ElfIdentityBuffer);

#endif