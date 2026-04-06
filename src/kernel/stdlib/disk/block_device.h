#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include "stdint.h"

typedef struct block_device {
  int (*read)(struct block_device *dev, uint32_t lba, uint8_t count,
              uint8_t *buf);
  int (*write)(struct block_device *dev, uint32_t lba, uint8_t count,
               const uint8_t *buf);

  int (*init)(void);

  uint32_t sector_size;
  void *driver_data;
} block_device_t;

#endif
