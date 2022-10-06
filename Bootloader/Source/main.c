#include <efi.h>
#include <efilib.h>

#include <stddef.h>

#include "elf_loader.h"

int memcmp(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
  
	return 0;
}

EFI_FILE_HANDLE GetVolume(EFI_HANDLE image) {
  EFI_LOADED_IMAGE *LoadedImage = NULL;
  EFI_GUID LoadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_FILE_IO_INTERFACE *IOVolume;
  EFI_GUID FileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_FILE_HANDLE Volume;
 
  uefi_call_wrapper(BS->HandleProtocol, 3, image, &LoadedImageGuid, (void **) &LoadedImage);

  uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &FileSystemGuid, (VOID*) &IOVolume);
  uefi_call_wrapper(IOVolume->OpenVolume, 2, IOVolume, &Volume);

  return Volume;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);

  Print(L"MyOS Bootloader\n");
  
  EFI_FILE_HANDLE Volume = GetVolume(ImageHandle);

  return EFI_SUCCESS;
}