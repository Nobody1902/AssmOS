#include "shell/shell.h"
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

  block_device_t *ata_device = create_ata_device();

  if (ata_device->init() != ATA_OK) {
    printf("ATA init failed.\n");
    return;
  }

  fat32_t fs;

  if (fat32_mount(&fs, ata_device) == 0) {
    printf("FAT32 mounted!\n");
    fat32_listdir(&fs, "/");
    uint32_t sz;
    fat32_file_size(&fs, "test.txt", &sz);
    printf("test.txt size = %u bytes\n", sz);

    char *buf = kmalloc(sz);
    fat32_read(&fs, "test.txt", buf, sz);

    buf[sz - 1] = '\0';

    printf("%s\n", buf);

    kfree(buf);
  } else {
    printf("Failed to mount FAT32\n");
  }

  shell();
}
