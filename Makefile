OS_NAME 				= MyOS

GNU_EFI_DIR 			= gnu-efi
OVMFBIN_DIR 			= OVMFbin

LDS 					= scripts/kernel.ld
CC 						= gcc
LD 						= ld

ROOT_BUILD_DIR			= build
SCRIPTS_DIR				= scripts

BOOTLOADER_CFLAGS 		= -I$(GNU_EFI_DIR)/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args
BOOTLOADER_LDFLAGS 		= -shared -Bsymbolic -L$(GNU_EFI_DIR)/x86_64/lib -L$(GNU_EFI_DIR)/x86_64/gnuefi -T$(GNU_EFI_DIR)/gnuefi/elf_x86_64_efi.lds $(GNU_EFI_DIR)/x86_64/gnuefi/crt0-efi-x86_64.o -lgnuefi -lefi

BOOTLOADER_SRC_DIR 		:= src/bootloader
BOOTLOADER_OBJ_DIR 		:= obj/bootloader
BOOTLOADER_BUILD_DIR 	= build/bootloader

KERNEL_CFLAGS 			= -ffreestanding -fshort-wchar
KERNEL_LDFLAGS 			= -T $(LDS) -static -Bsymbolic -nostdlib

KERNEL_SRC_DIR 			:= src/kernel
KERNEL_OBJ_DIR 			:= obj/kernel
KERNEL_BUILD_DIR 		= build/kernel

rwildcard				=	$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

BOOTLOADER_SRC 			=	$(call rwildcard,$(BOOTLOADER_SRC_DIR),*.c)          
BOOTLOADER_OBJS 		=	$(patsubst $(BOOTLOADER_SRC_DIR)/%.c, $(BOOTLOADER_OBJ_DIR)/%.o, $(BOOTLOADER_SRC))
BOOTLOADER_DIRS			=	$(wildcard $(BOOTLOADER_SRC_DIR)/*)

KERNEL_SRC 				=	$(call rwildcard,$(KERNEL_SRC_DIR),*.c)          
KERNEL_OBJS 			=	$(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_OBJ_DIR)/%.o, $(KERNEL_SRC))
KERNEL_DIRS				=	$(wildcard $(KERNEL_SRC_DIR)/*)

Bootloader: BootloaderSetup $(BOOTLOADER_OBJS) BootloaderLink BootloaderEFI

$(BOOTLOADER_OBJ_DIR)/%.o: $(BOOTLOADER_SRC_DIR)/%.c
	@ echo !==== COMPILING BOOTLOADER $^
	@ mkdir -p $(@D)
	$(CC) $(BOOTLOADER_CFLAGS) -c $^ -o $@
	
BootloaderLink:
	@ echo !==== LINKING BOOTLOADER
	$(LD) $(BOOTLOADER_LDFLAGS) -o $(BOOTLOADER_BUILD_DIR)/bootloader.so $(BOOTLOADER_OBJS) -lgnuefi -lefi

BootloaderEFI:
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(BOOTLOADER_BUILD_DIR)/bootloader.so $(BOOTLOADER_BUILD_DIR)/bootloader.efi

BootloaderSetup:
	@mkdir -p $(BOOTLOADER_BUILD_DIR)
	@mkdir -p $(BOOTLOADER_SRC_DIR)
	@mkdir -p $(BOOTLOADER_OBJ_DIR)

Kernel: KernelSetup $(KERNEL_OBJS) KernelLink

$(KERNEL_OBJ_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@ echo !==== COMPILING KERNEL $^
	@ mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $^ -o $@
	
KernelLink:
	@ echo !==== LINKING KERNEL
	$(LD) $(KERNEL_LDFLAGS) -o $(KERNEL_BUILD_DIR)/kernel.elf $(KERNEL_OBJS)

KernelSetup:
	@mkdir -p $(KERNEL_BUILD_DIR)
	@mkdir -p $(KERNEL_SRC_DIR)
	@mkdir -p $(KERNEL_OBJ_DIR)

BuildImage: Bootloader Kernel
	dd if=/dev/zero of=$(ROOT_BUILD_DIR)/$(OS_NAME).img bs=512 count=93750
	mformat -i $(ROOT_BUILD_DIR)/$(OS_NAME).img -f 1440 ::
	mmd -i $(ROOT_BUILD_DIR)/$(OS_NAME).img ::/EFI
	mmd -i $(ROOT_BUILD_DIR)/$(OS_NAME).img ::/EFI/BOOT
	mcopy -i $(ROOT_BUILD_DIR)/$(OS_NAME).img $(BOOTLOADER_BUILD_DIR)/bootloader.efi ::/EFI/BOOT
	mcopy -i $(ROOT_BUILD_DIR)/$(OS_NAME).img $(SCRIPTS_DIR)/startup.nsh ::
	mcopy -i $(ROOT_BUILD_DIR)/$(OS_NAME).img $(KERNEL_BUILD_DIR)/kernel.elf ::

Clean:
	rm -rf build/
	rm -rf obj/

Run: BuildImage
	qemu-system-x86_64 -drive file=$(ROOT_BUILD_DIR)/$(OS_NAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFBIN_DIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFBIN_DIR)/OVMF_VARS-pure-efi.fd" -net none
