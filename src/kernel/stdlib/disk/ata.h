#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// Primary bus ports
#define ATA_PRIMARY_BASE 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6 // Control / Alt-status

// Register offsets from base
#define ATA_REG_DATA 0x00     // R/W  16-bit
#define ATA_REG_ERROR 0x01    // R    error info after failure
#define ATA_REG_FEATURES 0x01 // W    features (write)
#define ATA_REG_SECTOR_COUNT 0x02
#define ATA_REG_LBA_LO 0x03
#define ATA_REG_LBA_MID 0x04
#define ATA_REG_LBA_HI 0x05
#define ATA_REG_DRIVE_HEAD 0x06
#define ATA_REG_STATUS 0x07  // R
#define ATA_REG_COMMAND 0x07 // W

// Status register bits
#define ATA_STATUS_ERR (1 << 0)
#define ATA_STATUS_DRQ (1 << 3)
#define ATA_STATUS_SRV (1 << 4)
#define ATA_STATUS_DF (1 << 5)
#define ATA_STATUS_RDY (1 << 6)
#define ATA_STATUS_BSY (1 << 7)

// Commands
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_IDENTIFY 0xEC

// Drive selectors
#define ATA_DRIVE_MASTER 0xE0
#define ATA_DRIVE_SLAVE 0xF0

// Sector size
#define ATA_SECTOR_SIZE 512

// Return codes
#define ATA_OK 0
#define ATA_ERR_NO_DRIVE -1
#define ATA_ERR_TIMEOUT -2
#define ATA_ERR_STATUS -3
#define ATA_ERR_NOT_ATA -4

/**
 * Initialise the ATA driver and detect the drive.
 *
 * @return ATA_OK on success, negative error code otherwise.
 */
int ata_init(void);

/**
 * Read @count sectors starting at @lba into @buf.
 * @buf must be at least count * ATA_SECTOR_SIZE bytes.
 *
 * @return ATA_OK on success, negative error code otherwise.
 */
int ata_read(uint32_t lba, uint8_t count, uint8_t *buf, uint8_t drive_selector);

/**
 * Write @count sectors from @buf to disk starting at @lba.
 * @buf must be at least count * ATA_SECTOR_SIZE bytes.
 *
 * @return ATA_OK on success, negative error code otherwise.
 */
int ata_write(uint32_t lba, uint8_t count, const uint8_t *buf,
              uint8_t drive_selector);

#endif
