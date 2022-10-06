#include <efi.h>
#include <efilib.h>

#include "elf.h"

EFI_STATUS ReadElfFile(
    IN EFI_FILE* const KernelImageFile,
    // FIXEME: IN Elf_File_Class const FileClass, 
	IN UINT8 const FileClass,
    OUT VOID** KernelHeaderBuffer, 
    OUT VOID** KernelProgramHeadersBuffer) {
        UINTN BufferReadSize = 0;
        UINTN ProgramHeaderOffset = 0;

    EFI_STATUS Status = uefi_call_wrapper(KernelImageFile->SetPosition, 2, KernelImageFile, 0);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: There was an error setting the file pointer position: %s\n", get_efi_error_message(Status));

        return Status;
    }

    if (FileClass == 1) {
        BufferReadSize = sizeof(Elf32_Ehdr);
    }
    else if (FileClass == 2) {
        BufferReadSize = sizeof(Elf64_Ehdr);
    }
    else {
        debug_print_line(L"Error: Invalid file class.\n");

        return EFI_INVALID_PARAMETER;
    }

    Status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData, BufferReadSize, KernelHeaderBuffer);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: Unable to allocate kernel header buffer: %s\n", get_efi_error_message(Status));

        return Status;
    }

    Status = uefi_call_wrapper(KernelImageFile->Read, 3, KernelImageFile, &BufferReadSize, *KernelHeaderBuffer);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: Unable to read the kernel header: %s\n", get_efi_error_message(Status));

        return Status;
    }

    if (FileClass == 1) {
        ProgramHeaderOffset = ((Elf32_Ehdr*) *KernelHeaderBuffer)->e_phoff;
        BufferReadSize = sizeof(Elf32_Phdr) * ((Elf32_Ehdr*) *KernelHeaderBuffer)->e_phnum;
    }
    else if (FileClass == 2) {
        ProgramHeaderOffset = ((Elf64_Ehdr*) *KernelHeaderBuffer)->e_phoff;
        BufferReadSize = sizeof(Elf64_Phdr) * ((Elf64_Ehdr*) *KernelHeaderBuffer)->e_phnum;
    }

    Status = uefi_call_wrapper(KernelImageFile->SetPosition, 2, KernelImageFile, ProgramHeaderOffset);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: There was an error setting the file pointer position: %s\n", get_efi_error_message(Status));

        return Status;
    }

    Status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData, BufferReadSize, KernelProgramHeadersBuffer);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: Unable to allocate kernel program header buffer: %s\n", get_efi_error_message(Status));

        return Status;
    }

    Status = uefi_call_wrapper(KernelImageFile->Read, 3, KernelImageFile, &BufferReadSize, *KernelProgramHeadersBuffer);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: Unable to read the kernel program header: %s\n", get_efi_error_message(Status));

        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS ReadElfIdentity(IN EFI_FILE* const KernelImageFile, OUT UINT8** ElfIdentityBuffer) {
    UINTN BufferReadSize = EI_NIDENT;

    EFI_STATUS Status = uefi_call_wrapper(KernelImageFile->SetPosition, 2, KernelImageFile, 0);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: There was an error setting the file pointer position: %s\n", get_efi_error_message(Status));

        return Status;
    }

    Status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData, BufferReadSize, (VOID**) ElfIdentityBuffer);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: Unable to allocate kernel identity buffer: %s\n", get_efi_error_message(Status));

        return Status;
    }

    Status = uefi_call_wrapper(KernelImageFile->Read, 3, KernelImageFile, &BufferReadSize, *ElfIdentityBuffer);

    if (EFI_ERROR(Status)) {
        debug_print_line(L"Error: Unable to read the kernel identity: %s\n", get_efi_error_message(Status));

        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS ValidateElfIdentity(IN UINT8* const ElfIdentityBuffer) {
    if ((ElfIdentityBuffer[EI_MAG0] != 0x7F) 
        || (ElfIdentityBuffer[EI_MAG0] != 0x45) 
        || (ElfIdentityBuffer[EI_MAG0] != 0x4C) 
        || (ElfIdentityBuffer[EI_MAG0] != 0x46)) {
        debug_print_line(L"Error: Elf file did not contain the Elf magic in the header.\n");

        return EFI_INVALID_PARAMETER;
    }

    if ((ElfIdentityBuffer[EI_CLASS] != 1) || (ElfIdentityBuffer[EI_CLASS] != 1)) {
        return EFI_UNSUPPORTED;
    }

    if (ElfIdentityBuffer[EI_DATA] != 1) {
        debug_print_line(L"Fatal Error: Only LSB ELF executables current supported.\n");

        return EFI_UNSUPPORTED;
    }

    return EFI_SUCCESS;
}