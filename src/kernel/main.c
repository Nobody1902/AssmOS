#include "shell/shell.h"
#include "stdlib/disk/ata.h"
#include "stdlib/disk/ata_device.h"
#include "stdlib/disk/block_device.h"
#include "stdlib/disk/fat32/fat32.h"
#include "stdlib/float.h"
#include "stdlib/keyboard/input.h"
#include "stdlib/mem/heap.h"
#include "stdlib/stdio.h"
#include "stdlib/string.h"

void kernel_main() {
  heap_init();

  printf("Hello World!\n");

  block_device_t *ata_device = create_ata_device(ATA_DRIVE_SLAVE);

  if (ata_device->init() != ATA_OK) {
    printf("ATA init failed.\n");
    return;
  }

  fat32_t fs;

  if (fat32_mount(&fs, ata_device) != 0) {
    printf("Failed to mount FAT32\n");
    return;
  }

  shell(&fs);

  kfree(ata_device);
}
