#ifndef BOOTLOADER_GRAPHICS_H
#define BOOTLOADER_GRAPHICS_H 1

#include <efi.h>
#include <efilib.h>

typedef struct s_uefi_graphics_service {
	EFI_HANDLE* handle_buffer;
	UINTN handle_count;
} UEFIGraphicsService;

VOID DrawRect(IN EFI_GRAPHICS_OUTPUT_PROTOCOL* const protocol,
	IN const UINT16 _x,
	IN const UINT16 _y,
	IN const UINT16 _width,
	IN const UINT16 _height,
	IN const UINT32 color);

EFI_STATUS CloseGraphicOutputService(void);

EFI_STATUS InitGraphicsOutputService(void);

EFI_STATUS SetGraphicsMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL* const protocol,
	IN const UINT32 target_width,
	IN const UINT32 target_height,
	IN const EFI_GRAPHICS_PIXEL_FORMAT target_pixel_format);

#endif