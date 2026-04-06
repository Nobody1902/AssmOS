#include "ata_device.h"

block_device_t *create_ata_device() {
  static block_device_t dev;

  dev.read = ata_block_read;
  dev.write = ata_block_write;
  dev.init = ata_init;
  dev.sector_size = 512;
  dev.driver_data = 0;

  return &dev;
}
