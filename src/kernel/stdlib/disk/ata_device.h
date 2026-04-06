#ifndef ATA_DEVICE_H
#define ATA_DEVICE_H

#include "ata.h"
#include "block_device.h"

static int ata_block_read(block_device_t *dev, uint32_t lba, uint8_t count,
                          uint8_t *buf) {
  return ata_read(lba, count, buf);
}

static int ata_block_write(block_device_t *dev, uint32_t lba, uint8_t count,
                           const uint8_t *buf) {
  return ata_write(lba, count, buf);
}

block_device_t *create_ata_device();

#endif
