#ifndef UTILS_H 
#define UTILS_H 1

#include <efi.h>
#include <efilib.h>


EFI_STATUS GetMemoryMap(OUT VOID** MemoryMap,
	OUT UINTN* MemoryMapSize,
	OUT UINTN* MemoryMapKey,
	OUT UINTN* Descriptor,
	OUT UINT32* DescriptorVersion);

#endif