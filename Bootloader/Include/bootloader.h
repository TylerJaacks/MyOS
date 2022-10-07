#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <efi.h>
#include <efilib.h>

#include "fs.h"
#include "graphics.h"
#include "serial.h"

#define KERNEL_EXECUTABLE_PATH L"\\Kernel.elf"

#define TARGET_SCREEN_WIDTH 1024
#define TARGET_SCREEN_HEIGHT 768

#define TARGET_PIXEL_FORMAT PixelBlueGreenRedReserved8BitPerColor

typedef struct s_boot_video_info
{
    VOID *framebuffer_pointer;
    UINT32 horizontal_resolution;
    UINT32 vertical_resolution;
    UINT32 pixels_per_scaline;
} KernelBootVideoModeInfo;

typedef struct s_boot_info
{
    EFI_MEMORY_DESCRIPTOR *memory_map;
    UINTN memory_map_size;
    UINTN memory_map_descriptor_size;
    KernelBootVideoModeInfo video_mode_info;
} KernelBootInfo;

UEFIGraphicsService GraphicsService;
UEFIFileSystemService FileSystemService;
UEFISerialService SerialService;

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

#endif BOOTLOADER_H