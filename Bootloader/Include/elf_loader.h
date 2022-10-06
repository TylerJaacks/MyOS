#ifndef BOOTLOADER_ELF_LOADER_H
#define BOOTLOADER_ELF_LOADER_H 1

#include <efi.h>
#include <efilib.h>

EFI_STATUS LoadSegment(
    IN EFI_FILE* const KernelImageFile,
    IN EFI_PHYSICAL_ADDRESS const SegmentFileOffset,
    IN UINTN const SegmentFileSize,
    IN UINTN const SegmentMemorySize,
    IN EFI_PHYSICAL_ADDRESS const SegmentPhysicalAddress);

EFI_STATUS LoadProgramSegments(
    IN EFI_FILE* const KernelImageFile,
    IN UINTN const FileClass,
    IN VOID* const KernelHeaderBuffer,
    IN VOID* const KernelProgramHeadersBuffer);

EFI_STATUS LoadKernelImage(IN EFI_FILE* const RootFileSystem,
	IN CHAR16* const KernelImageFilename,
	OUT EFI_PHYSICAL_ADDRESS* KernelEntryPoint);

#endif // BOOTLOADER_ELF_LOADER_H