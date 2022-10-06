#ifndef BOOTLOADER_DEBUG_H
#define BOOTLOADER_DEBUG_H 1

EFI_STATUS debug_print_line(IN CHAR16* fmt, ...)
{
	#define MAX_LENGTH 64 * 1024
	
	EFI_STATUS status;
	
	va_list args;

	CHAR16 output_message[MAX_LENGTH];

	va_start(args, fmt);

	VPrint(fmt, args);

	va_end(args);

	return EFI_SUCCESS;
};

#endif