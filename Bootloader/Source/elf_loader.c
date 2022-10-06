#include "elf_loader.h"

EFI_STATUS LoadSegment(
    IN EFI_FILE* const KernelImageFile,
    IN EFI_PHYSICAL_ADDRESS const SegmentFileOffset,
    IN UINTN const SegmentFileSize,
    IN UINTN const SegmentMemorySize,
    IN EFI_PHYSICAL_ADDRESS const SegmentPhysicalAddress) {
    
    EFI_STATUS Status;
    VOID* ProgramData = NULL;
    UINTN BufferReadSize = 0;
    UINTN SegmentPageCount = EFI_SIZE_TO_PAGES(SegmentMemorySize);
    EFI_PHYSICAL_ADDRESS ZeroFillStart = 0;
    UINTN ZeroFillCount = 0;

    Status = uefi_call_wrapper(KernelImageFile->SetPosition, 2, KernelImageFile, SegmentFileOffset);

    if (check_for_fatal_error(Status, L"Error setting file pointer to segment offset.\n")) {
		return Status;
	}

    Status = uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAddress, EfiLoaderData, SegmentPageCount, (EFI_PHYSICAL_ADDRESS*) &SegmentPhysicalAddress);

    if (check_for_fatal_error(Status, L"Error allocating pages for ELF segment.\n")) {

		return Status;
	}

    if (SegmentFileSize > 0) {
        BufferReadSize = SegmentFileSize;

        Status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderCode, BufferReadSize, (VOID**) &ProgramData);
        
        if (check_for_fatal_error(Status, L"Error allocating kernel segment buffer.\n")) {
            return Status;
        }

        Status = uefi_call_wrapper(KernelImageFile->Read, 3, KernelImageFile, &BufferReadSize, (VOID*) ProgramData);

        if (check_for_fatal_error(Status, L"Error reading segment data.\n")) {
			return Status;
		}

        Status = uefi_call_wrapper(gBS->CopyMem, 3, SegmentPhysicalAddress, ProgramData, SegmentFileSize);

        if (check_for_fatal_error(Status, L"Error copying program section into memory.\n")) {
			return Status;
		}

        Status = uefi_call_wrapper(gBS->FreePool, 1, ProgramData);

        if (check_for_fatal_error(Status, L"Error freeing program section.\n")) {
			return Status;
		}
    }

    ZeroFillStart = SegmentPhysicalAddress + SegmentFileOffset;
    ZeroFillCount = SegmentMemorySize - SegmentFileOffset;

    if (ZeroFillCount > 0) {
        Status = uefi_call_wrapper(gBS->SetMem, 3, ZeroFillStart, ZeroFillCount, 0);

        if (check_for_fatal_error(Status, L"Error zero filling segment")) {
			return Status;
		}
    }
}