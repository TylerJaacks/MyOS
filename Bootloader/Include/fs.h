#ifndef BOOTLOADER_FS_H
#define BOOTLOADER_FS_H 1

#include <efi.h>
#include <efilib.h>

typedef struct s_uefi_simple_file_system_service {
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* protocol;
} UEFIFileSystemService;

EFI_STATUS InitFileSystemService(void);

#endif