OS_NAME 				= MyOS

GNU_EFI_DIR 			= gnu-efi
OVMFBIN_DIR 			= OVMFbin
LDS 					= scripts/kernel.ld
CC 						= gcc
LD 						= ld

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

BOOT_EFI 				:= $(GNU_EFI_DIR)/x86_64/bootloader/main.efi

rwildcard				=	$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

BOOTLOADER_SRC 			=	$(call rwildcard,$(BOOTLOADER_SRC_DIR),*.c)          
BOOTLOADER_OBJS 		=	$(patsubst $(BOOTLOADER_SRC_DIR)/%.c, $(BOOTLOADER_OBJ_DIR)/%.o, $(BOOTLOADER_SRC))
BOOTLOADER_DIRS			=	$(wildcard $(BOOTLOADER_SRC_DIR)/*)

KERNEL_SRC 				=	$(call rwildcard,$(KERNEL_SRC_DIR),*.c)          
KERNEL_OBJS 			=	$(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_OBJ_DIR)/%.o, $(KERNEL_SRC))
KERNEL_DIRS				=	$(wildcard $(KERNEL_SRC_DIR)/*)

Bootloader: BootloaderSetup $(BOOTLOADER_OBJS) BootloaderLink

$(BOOTLOADER_OBJ_DIR)/%.o: $(BOOTLOADER_SRC_DIR)/%.c
	@ echo !==== COMPILING BOOTLOADER $^
	@ mkdir -p $(@D)
	$(CC) $(BOOTLOADER_CFLAGS) -c $^ -o $@
	
BootloaderLink:
	@ echo !==== LINKING BOOTLOADER
	$(LD) $(BOOTLOADER_LDFLAGS) -o $(BOOTLOADER_BUILD_DIR)/bootloader.elf $(BOOTLOADER_OBJS)

BootloaderSetup:
	@mkdir -p $(BOOTLOADER_BUILD_DIR)/bootloader
	@mkdir -p $(BOOTLOADER_SRC_DIR)
	@mkdir -p $(BOOTLOADER_OBJ_DIR)/bootloader

Kernel: KernelSetup $(KERNEL_OBJS) KernelLink

$(KERNEL_OBJ_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@ echo !==== COMPILING KERNEL $^
	@ mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $^ -o $@
	
KernelLink:
	@ echo !==== LINKING KERNEL
	$(LD) $(KERNEL_LDFLAGS) -o $(KERNEL_BUILD_DIR)/kernel.elf $(KERNEL_OBJS)

KernelSetup:
	@mkdir -p $(KERNEL_BUILD_DIR)/kernel
	@mkdir -p $(KERNEL_SRC_DIR)
	@mkdir -p $(KERNEL_OBJ_DIR)/kernel

BuildImage:
	dd if=/dev/zero of=$(BUILDDIR)/$(OSNAME).img bs=512 count=93750
	mformat -i $(BUILDDIR)/$(OSNAME).img -f 1440 ::
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BOOTEFI) ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/$(OSNAME).img startup.nsh ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/kernel.elf ::

Clean:
	rm -rf build/
	rm -rf obj/

Run:
	qemu-system-x86_64 -drive file=$(BUILDDIR)/$(OSNAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFBIN_DIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFBIN_DIR)/OVMF_VARS-pure-efi.fd" -net none
