# Directories
BASE_DIR := $(CURDIR)
ISO_DIR := $(BASE_DIR)/iso
BIN_DIR := $(BASE_DIR)/bin
OBJECT_DIR := $(BIN_DIR)/object
SRC_DIR := $(BASE_DIR)/src

KERNEL_OBJ_DIR := $(OBJECT_DIR)/kernel
BOOT_OBJ_DIR := $(OBJECT_DIR)/boot

# Tools
CC := x86_64-elf-gcc
AS := nasm
LD := ld

CFLAGS := -ffreestanding -m64 -msse -msse2 -mfpmath=sse
ASFLAGS := -f elf64

# Sources
KERNEL_SRC_C := $(shell find $(SRC_DIR)/kernel -name "*.c")
KERNEL_SRC_ASM := $(shell find $(SRC_DIR)/kernel -name "*.asm")
BOOT_SRC := $(shell find $(SRC_DIR)/boot -name "*.asm")

# Objects
KERNEL_OBJ_C := $(patsubst $(SRC_DIR)/kernel/%.c,$(KERNEL_OBJ_DIR)/%.o,$(KERNEL_SRC_C))
KERNEL_OBJ_ASM := $(patsubst $(SRC_DIR)/kernel/%.asm,$(KERNEL_OBJ_DIR)/%.o,$(KERNEL_SRC_ASM))
BOOT_OBJ := $(patsubst $(SRC_DIR)/boot/%.asm,$(BOOT_OBJ_DIR)/%.o,$(BOOT_SRC))

ALL_KERNEL_OBJ := $(KERNEL_OBJ_C) $(KERNEL_OBJ_ASM)

# Final outputs
KERNEL_BIN := $(BIN_DIR)/kernel.bin
ISO_IMAGE := $(BIN_DIR)/system.iso
DISK_IMAGE := $(BIN_DIR)/disk.img

INIT_FS_DIR := ${SRC_DIR}/initfs
INIT_FS := $(wildcard $(INIT_FS_DIR)/**/*) $(wildcard $(INIT_FS_DIR)/*)

# Default target
all: $(ISO_IMAGE) $(DISK_IMAGE)

# Create directories
$(BIN_DIR) $(OBJECT_DIR) $(KERNEL_OBJ_DIR) $(BOOT_OBJ_DIR):
	@mkdir -p $@

# Compile C kernel sources
$(KERNEL_OBJ_DIR)/%.o: $(SRC_DIR)/kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

# Compile ASM kernel sources
$(KERNEL_OBJ_DIR)/%.o: $(SRC_DIR)/kernel/%.asm
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

# Compile boot ASM sources
$(BOOT_OBJ_DIR)/%.o: $(SRC_DIR)/boot/%.asm
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

# Link kernel + bootloader
$(KERNEL_BIN): $(ALL_KERNEL_OBJ) $(BOOT_OBJ) | $(BIN_DIR)
	$(LD) -n -o $@ -T $(BASE_DIR)/linker.ld $(BOOT_OBJ) $(ALL_KERNEL_OBJ)

# Build ISO
$(ISO_IMAGE): $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	grub-mkrescue /usr/lib/grub/i386-pc -o $@ $(ISO_DIR)

# Create disk image
$(DISK_IMAGE): $(INIT_FS)
	dd if=/dev/zero of=$@ bs=1M count=64
	mkfs.fat -F 32 $@

	mcopy -i $@ -s $(INIT_FS_DIR)/* ::

# Run in QEMU
run: all
	qemu-system-x86_64 \
		-boot d \
		-drive file=$(ISO_IMAGE),format=raw,media=cdrom,if=ide \
		-drive file=$(DISK_IMAGE),format=raw,media=disk,if=ide

# Clean
clean:
	rm -rf $(BIN_DIR)

.PHONY: all run clean
