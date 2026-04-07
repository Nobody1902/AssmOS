#ifndef ATA_DEVICE_H
#define ATA_DEVICE_H

#include "../mem/heap.h"
#include "../string.h"
#include "ata.h"
#include "block_device.h"

typedef struct {
  uint8_t drive_selector;
} ata_drv_t;

static int ata_block_read(block_device_t *dev, uint32_t lba, uint8_t count,
                          uint8_t *buf) {
  ata_drv_t *ata = (ata_drv_t *)dev->driver_data;
  if (!ata)
    return -1;
  return ata_read(lba, count, buf, ata->drive_selector) == ATA_OK ? 0 : -1;
}

static int ata_block_write(block_device_t *dev, uint32_t lba, uint8_t count,
                           const uint8_t *buf) {
  ata_drv_t *ata = (ata_drv_t *)dev->driver_data;
  if (!ata)
    return -1;
  return ata_write(lba, count, buf, ata->drive_selector) == ATA_OK ? 0 : -1;
}

block_device_t *create_ata_device(uint8_t drive_selector);

#endif
