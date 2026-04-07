#include "ata_device.h"

block_device_t *create_ata_device(uint8_t drive_selector) {
  block_device_t *dev = kmalloc(sizeof(block_device_t));
  if (!dev)
    return NULL;

  ata_drv_t *ata_drv = kmalloc(sizeof(ata_drv_t));
  if (!ata_drv) {
    kfree(dev);
    return NULL;
  }

  ata_drv->drive_selector = drive_selector;

  dev->read = ata_block_read;
  dev->write = ata_block_write;
  dev->init = ata_init;
  dev->sector_size = 512;
  dev->driver_data = ata_drv;

  return dev;
}
