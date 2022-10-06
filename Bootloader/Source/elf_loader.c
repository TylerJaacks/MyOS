#include "elf_loader.h"
#include "elf.h"

#include "debug.h"

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

EFI_STATUS LoadProgramSegments(
    IN EFI_FILE* const KernelImageFile,
    IN UINTN const FileClass,
    IN VOID* const KernelHeaderBuffer,
    IN VOID* const KernelProgramHeadersBuffer) {
    
    EFI_STATUS Status;
    UINT16 NumberOfProgramHeaders = 0;
    UINT16 NumberOfSegmentsLoaded = 0;
    UINTN p = 0;

    if (FileClass == 1) {
        NumberOfProgramHeaders = ((Elf32_Ehdr*) KernelHeaderBuffer)->e_phnum;
    }
    else if (FileClass == 2) {
        NumberOfProgramHeaders = ((Elf64_Ehdr*) KernelHeaderBuffer)->e_phnum;
    }

    if (NumberOfProgramHeaders == 0) {
        debug_print_line(L"Fatal Error: No program segments to load.");

		return EFI_INVALID_PARAMETER;
    }

    if (FileClass == 1) {
        Elf32_Phdr* ProgramHeaders = (Elf32_Ehdr*) KernelProgramHeadersBuffer;

        for (p = 0; p < NumberOfProgramHeaders; p++) {
            if (ProgramHeaders[p].p_type == PT_LOAD) {
				Status = LoadSegment(KernelImageFile,
					ProgramHeaders[p].p_offset,
					ProgramHeaders[p].p_filesz,
					ProgramHeaders[p].p_memsz,
					ProgramHeaders[p].p_paddr);
            }

            if (EFI_ERROR(Status)) {
                return Status;
            }

            NumberOfSegmentsLoaded++;
        }
    }
    else if (FileClass == 2) {
        Elf64_Phdr* ProgramHeaders = (Elf64_Ehdr*) KernelProgramHeadersBuffer;

        for (p = 0; p < NumberOfProgramHeaders; p++) {
            if (ProgramHeaders[p].p_type == PT_LOAD) {
				Status = LoadSegment(KernelImageFile,
					ProgramHeaders[p].p_offset,
					ProgramHeaders[p].p_filesz,
					ProgramHeaders[p].p_memsz,
					ProgramHeaders[p].p_paddr);
            }

            if (EFI_ERROR(Status)) {
                return Status;
            }

            NumberOfSegmentsLoaded++;
        }
    }

    if (NumberOfSegmentsLoaded == 0) {
        debug_print_line(L"Fatal Error: No loadable program segments.");

		return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}

EFI_STATUS LoadKernelImage(IN EFI_FILE* const RootFileSystem,
	IN CHAR16* const KernelImageFilename,
	OUT EFI_PHYSICAL_ADDRESS* KernelEntryPoint) {
    
    EFI_STATUS Status;
    EFI_FILE* KernelImageFile;
    VOID* KernelHeader = NULL;
    VOID* KernelProgramHeaders = NULL;
    UINT8* ElfIdentityBuffer = NULL;
    Elf_File_Class FileClass = ELF_FILE_CLASS_NONE;

    // Open the Kernel Elf Image.
    Status = uefi_call_wrapper(RootFileSystem->Open, 5, RootFileSystem, &KernelImageFile, KernelImageFilename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

    if (check_for_fatal_error(Status, L"Error opening Kernel file.")) {
        return Status;
    }

    // Reading the Kernel Elf Image Identity.
    Status = ReadElfIdentity(KernelImageFile, &ElfIdentityBuffer);

    if (check_for_fatal_error(Status, L"Error reading Kernel elf identity.")) {
        return Status;
    }

    FileClass = ElfIdentityBuffer[EI_CLASS];

    // Validate the Kernel Elf Image.
    Status = ValidateElfIdentity(ElfIdentityBuffer);

    if (check_for_fatal_error(Status, L"Error validating the Kernel Elf Image.")) {
        return Status;
    }

    // Free the Kernel Elf Image Idenity Buffer/Pool.
    Status = uefi_call_wrapper(gBS->FreePool, 1, ElfIdentityBuffer);

    if (check_for_fatal_error(Status, L"Unable to free the Kernel Elf Image Identity Pool.")) {
        return Status;
    }

    if (FileClass == ELF_FILE_CLASS_32) {
        *KernelEntryPoint = ((Elf32_Ehdr*) KernelHeader)->e_entry;
    }
    else if (FileClass == ELF_FILE_CLASS_64) {
        *KernelEntryPoint = ((Elf64_Ehdr*) KernelHeader)->e_entry;
    }

    Status = LoadProgramSegments(KernelImageFile, FileClass, KernelHeader, KernelProgramHeaders);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = uefi_call_wrapper(KernelImageFile->Close, 1, KernelImageFile);

    if (check_for_fatal_error(Status, L"Unable to close the Kernel Elf Image.")) {
        return Status;
    }

	Status = uefi_call_wrapper(gBS->FreePool, 1, (VOID*) KernelHeader);

	if (check_for_fatal_error(Status, L"Error freeing kernel header buffer.")) {
		return Status;
	}

	Status = uefi_call_wrapper(gBS->FreePool, 1, (VOID*) KernelProgramHeaders);

	if (check_for_fatal_error(Status, L"Error freeing kernel program headers buffer.")) {
		return Status;
	}


    return EFI_SUCCESS;
}