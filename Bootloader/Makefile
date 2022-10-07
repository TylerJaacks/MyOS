OS_NAME 				= MyOS

GNU_EFI_DIR 			= ../gnu-efi
OVMFBIN_DIR 			= ../OVMFbin

LDS 					= ../Meta/Scripts/Kernel.ld
CC 						= gcc
LD 						= ld

ROOT_BUILD_DIR			= ../Build
SCRIPTS_DIR				= ../Meta/Scripts

BOOTLOADER_CFLAGS 		= -IInclude -I$(GNU_EFI_DIR)/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args
BOOTLOADER_LDFLAGS 		= -shared -Bsymbolic -L$(GNU_EFI_DIR)/x86_64/lib -L$(GNU_EFI_DIR)/x86_64/gnuefi -T$(GNU_EFI_DIR)/gnuefi/elf_x86_64_efi.lds $(GNU_EFI_DIR)/x86_64/gnuefi/crt0-efi-x86_64.o -lgnuefi -lefi


BOOTLOADER_SRC_DIR 			:= .
BOOTLOADER_OBJ_DIR 			:= ../Build/Bootloader
BOOTLOADER_BUILD_DIR 		= ../Build/Bootloader

rwildcard				=	$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

BOOTLOADER_SRC 				=	$(call rwildcard,$(BOOTLOADER_SRC_DIR),*.c)          
BOOTLOADER_OBJS 			=	$(patsubst $(BOOTLOADER_SRC_DIR)/%.c, $(BOOTLOADER_OBJ_DIR)/%.o, $(BOOTLOADER_SRC))
BOOTLOADER_DIRS				=	$(wildcard $(BOOTLOADER_SRC_DIR)/*)

Bootloader: BootloaderSetup $(BOOTLOADER_OBJS) BootloaderLink BootloaderEFI

$(BOOTLOADER_OBJ_DIR)/%.o: $(BOOTLOADER_SRC_DIR)/%.c
	@ echo !==== COMPILING BOOTLOADER $^
	@ mkdir -p $(@D)
	$(CC) $(BOOTLOADER_CFLAGS) -c $^ -o $@
	
BootloaderLink: $(BOOTLOADER_OBJS)
	@ echo !==== LINKING BOOTLOADER
	$(LD) $(BOOTLOADER_LDFLAGS) -o $(BOOTLOADER_BUILD_DIR)/main.so $(BOOTLOADER_OBJS) -lgnuefi -lefi

BootloaderEFI:
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(BOOTLOADER_BUILD_DIR)/main.so $(BOOTLOADER_BUILD_DIR)/BOOTX64.efi

BootloaderSetup:
	@mkdir -p $(BOOTLOADER_BUILD_DIR)
	@mkdir -p $(BOOTLOADER_SRC_DIR)
	@mkdir -p $(BOOTLOADER_OBJ_DIR)

Clean:
	rm -rf ../Build/Bootloader