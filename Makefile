OS_NAME 				= MyOS

GNU_EFI_DIR 			= gnu-efi
OVMFBIN_DIR 			= OVMFbin

ROOT_BUILD_DIR			= Build
SCRIPTS_DIR				= Meta/Scripts

# Build: Bootloader Kernel
# 	@ echo !=== BUILDING MyOS	

BuildImage:
	dd if=/dev/zero of=$(ROOT_BUILD_DIR)/$(OS_NAME).img bs=512 count=93750
	mformat -i $(ROOT_BUILD_DIR)/$(OS_NAME).img ::
	mmd -i $(ROOT_BUILD_DIR)/$(OS_NAME).img ::/EFI
	mmd -i $(ROOT_BUILD_DIR)/$(OS_NAME).img ::/EFI/BOOT
	mcopy -i $(ROOT_BUILD_DIR)/$(OS_NAME).img $(ROOT_BUILD_DIR)/Bootloader/main.efi ::/EFI/BOOT
	mcopy -i $(ROOT_BUILD_DIR)/$(OS_NAME).img $(SCRIPTS_DIR)/startup.nsh ::
	mcopy -i $(ROOT_BUILD_DIR)/$(OS_NAME).img $(ROOT_BUILD_DIR)/Kernel/Kernel.elf ::

Clean:
	rm -rf Build/

Run: BuildImage
	qemu-system-x86_64 -drive file=$(ROOT_BUILD_DIR)/$(OS_NAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFBIN_DIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFBIN_DIR)/OVMF_VARS-pure-efi.fd" -net none
