OS_NAME 				= MyOS

GNU_EFI_DIR 			= ../gnu-efi
OVMFBIN_DIR 			= ../OVMFbin

LDS 					= ../Meta/Scripts/Kernel.ld
CC 						= gcc
LD 						= ld

ROOT_BUILD_DIR			= ../Build
SCRIPTS_DIR				= ../Meta/Scripts

KERNEL_CFLAGS 			= -ffreestanding -fshort-wchar
KERNEL_LDFLAGS 			= -T $(LDS) -static -Bsymbolic -nostdlib

KERNEL_SRC_DIR 			:= .
KERNEL_OBJ_DIR 			:= ../Build/Kernel
KERNEL_BUILD_DIR 		= ../Build/Kernel

rwildcard				=	$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

KERNEL_SRC 				=	$(call rwildcard,$(KERNEL_SRC_DIR),*.c)          
KERNEL_OBJS 			=	$(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_OBJ_DIR)/%.o, $(KERNEL_SRC))
KERNEL_DIRS				=	$(wildcard $(KERNEL_SRC_DIR)/*)

Kernel: KernelSetup $(KERNEL_OBJS) KernelLink

$(KERNEL_OBJ_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@ echo !==== COMPILING KERNEL $^
	@ mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $^ -o $@
	
KernelLink:
	@ echo !==== LINKING KERNEL
	$(LD) $(KERNEL_LDFLAGS) -o $(KERNEL_BUILD_DIR)/Kernel.elf $(KERNEL_OBJS)

KernelSetup:
	@mkdir -p $(KERNEL_BUILD_DIR)
	@mkdir -p $(KERNEL_SRC_DIR)
	@mkdir -p $(KERNEL_OBJ_DIR)

Clean:
	rm -rf ../Build/Kernel