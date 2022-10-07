#include "utils.h"

EFI_STATUS GetMemoryMap(OUT VOID **MemoryMap,
                        OUT UINTN *MemoryMapSize,
                        OUT UINTN *MemoryMapKey,
                        OUT UINTN *DescriptorSize,
                        OUT UINT32 *DescriptorVersion)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY InputKey;

    Status = uefi_call_wrapper(gBS->GetMemoryMap, 5,
                               MemoryMapSize,
                               *MemoryMap,
                               MemoryMapKey,
                               DescriptorSize,
                               DescriptorVersion);

    if (EFI_ERROR(Status))
    {
        if (Status != EFI_BUFFER_TOO_SMALL)
        {
            return Status;
        }
    }

    *MemoryMapSize += 2 * (*DescriptorSize);

    Status = uefi_call_wrapper(gBS->AllocatePool, 3,
                               EfiLoaderData, *MemoryMapSize, (VOID **)MemoryMap);

    if (check_for_fatal_error(Status, L"Error allocating memory map buffer."))
    {
        return Status;
    }

    Status = uefi_call_wrapper(gBS->GetMemoryMap, 5,
                               MemoryMapSize,
                               *MemoryMap,
                               MemoryMapKey,
                               DescriptorSize,
                               DescriptorVersion);

    if (check_for_fatal_error(Status, L"Error getting memory map"))
    {
        return Status;
    }

    return EFI_SUCCESS;
}