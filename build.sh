#!/bin/bash

export BASE_DIR="$(pwd)"
export ISO_DIR="$(pwd)/iso"
export BIN_DIR="$(pwd)/bin"
export OBJECT_DIR="${BIN_DIR}/object"
export SRC_DIR="$(pwd)/src"

mkdir -p ${BIN_DIR}
mkdir -p ${OBJECT_DIR}

cd ${OBJECT_DIR}

echo "Compiling kernel and bootloader"
mkdir -p kernel
mkdir -p boot

export KERNEL_SRC_C="$(find ${SRC_DIR}/kernel -name "*.c")"
export KERNEL_SRC_ASM="$(find ${SRC_DIR}/kernel -name "*.asm")"
export BOOT_SRC="$(find ${SRC_DIR}/boot -name "*.asm")"

echo "Compiling ${KERNEL_SRC_C}"
echo "${KERNEL_SRC_ASM}"

cd "${OBJECT_DIR}/kernel" || exit
gcc -c ${KERNEL_SRC_C} -ffreestanding -m64
for item in $KERNEL_SRC_ASM; do
  export kernel_obj=$(basename $item)
  nasm -f elf64 $item -o $(pwd)/${kernel_obj%.*}.o
done

echo "Compiling ${BOOT_SRC}"

cd "${OBJECT_DIR}/boot" || exit
for item in $BOOT_SRC; do
  export boot_obj=$(basename $item)
  nasm -f elf64 $item -o $(pwd)/${boot_obj%.*}.o
done

echo "Linking kernel and bootloader"
cd "${BIN_DIR}" || exit

export kernel_obj="$(find $OBJECT_DIR/kernel -name "*.o")"
export boot_obj="$(find $OBJECT_DIR/boot -name "*.o")"
echo "$kernel_obj"
echo "$boot_obj"
ld -n -o kernel.bin -T "${BASE_DIR}/linker.ld" $boot_obj $kernel_obj
rm $kernel_obj
rm $boot_obj

cp kernel.bin "${ISO_DIR}/boot/"
grub-mkrescue /usr/lib/grub/i386-pc -o system.iso "${ISO_DIR}"

echo "Created ${BIN_DIR}/system.iso"

if [[ $1 = "run" ]]; then
  qemu-system-x86_64 -drive format=raw,file="${BIN_DIR}/system.iso"
  exit
fi

echo "Run: \"qemu-system-x86_64 -drive format=raw,file=${BIN_DIR}/system.iso\""
