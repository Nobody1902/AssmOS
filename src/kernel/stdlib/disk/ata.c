#include "ata.h"
#include "../port.h"

// Read the alternate status register (doesn't clear interrupts).
static inline uint8_t ata_alt_status(void) { return inb(ATA_PRIMARY_CTRL); }

// Required by ATA
static void ata_delay400(void) {
  ata_alt_status();
  ata_alt_status();
  ata_alt_status();
  ata_alt_status();
}

/**
 * Poll until BSY clears.
 * If check_drq is non-zero, also wait for DRQ to set afterwards.
 *
 * Returns ATA_OK, ATA_ERR_TIMEOUT, or ATA_ERR_STATUS.
 */
static int ata_poll(int check_drq) {
  uint32_t timeout = 100000;
  uint8_t status;

  do {
    status = inb(ATA_PRIMARY_BASE + ATA_REG_STATUS);
    if (--timeout == 0)
      return ATA_ERR_TIMEOUT;
  } while (status & ATA_STATUS_BSY);

  if (status & (ATA_STATUS_DF | ATA_STATUS_ERR))
    return ATA_ERR_STATUS;

  if (check_drq && !(status & ATA_STATUS_DRQ))
    return ATA_ERR_STATUS;

  return ATA_OK;
}

static int ata_select_drive(uint8_t drive_selector) {
  outb(ATA_PRIMARY_BASE + ATA_REG_DRIVE_HEAD, drive_selector);
  ata_delay400();
  return ata_poll(0);
}

int ata_init(void) {
  // Disable interrupts on the primary bus (we use PIO polling)
  outb(ATA_PRIMARY_CTRL, 0x02);

  // Primary slave
  outb(ATA_PRIMARY_BASE + ATA_REG_DRIVE_HEAD, ATA_DRIVE_SLAVE);
  ata_delay400();

  uint8_t status = ata_alt_status();
  if (status == 0xFF)
    return ATA_ERR_NO_DRIVE;

  // Send IDENTIFY
  outb(ATA_PRIMARY_BASE + ATA_REG_SECTOR_COUNT, 0);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_LO, 0);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_MID, 0);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_HI, 0);
  outb(ATA_PRIMARY_BASE + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
  ata_delay400();

  status = ata_alt_status();
  if (status == 0)
    return ATA_ERR_NO_DRIVE;

  // Wait for BSY to clear
  uint32_t timeout = 100000;
  while ((ata_alt_status() & ATA_STATUS_BSY) && --timeout)
    ;
  if (timeout == 0)
    return ATA_ERR_TIMEOUT;

  if (inb(ATA_PRIMARY_BASE + ATA_REG_LBA_MID) != 0 ||
      inb(ATA_PRIMARY_BASE + ATA_REG_LBA_HI) != 0)
    return ATA_ERR_NOT_ATA;

  timeout = 100000;
  while (!(ata_alt_status() & ATA_STATUS_DRQ) && --timeout)
    ;
  if (timeout == 0)
    return ATA_ERR_TIMEOUT;

  uint16_t identity[256];
  insw(ATA_PRIMARY_BASE + ATA_REG_DATA, identity, 256);

  return ATA_OK;
}

int ata_read(uint32_t lba, uint8_t count, uint8_t *buf) {
  int err;

  err = ata_select_drive(ATA_DRIVE_SLAVE | ((lba >> 24) & 0x0F));
  if (err != ATA_OK)
    return err;

  outb(ATA_PRIMARY_BASE + ATA_REG_SECTOR_COUNT, count);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_LO, (lba) & 0xFF);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_HI, (lba >> 16) & 0xFF);

  outb(ATA_PRIMARY_BASE + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

  for (int i = 0; i < count; i++) {
    ata_delay400();

    err = ata_poll(1);
    if (err != ATA_OK)
      return err;

    insw(ATA_PRIMARY_BASE + ATA_REG_DATA, buf + (i * ATA_SECTOR_SIZE), 256);
  }

  return ATA_OK;
}

int ata_write(uint32_t lba, uint8_t count, const uint8_t *buf) {
  int err;

  err = ata_select_drive(ATA_DRIVE_SLAVE | ((lba >> 24) & 0x0F));
  if (err != ATA_OK)
    return err;

  outb(ATA_PRIMARY_BASE + ATA_REG_SECTOR_COUNT, count);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_LO, (lba) & 0xFF);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
  outb(ATA_PRIMARY_BASE + ATA_REG_LBA_HI, (lba >> 16) & 0xFF);

  outb(ATA_PRIMARY_BASE + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

  for (int i = 0; i < count; i++) {
    ata_delay400();

    err = ata_poll(1);
    if (err != ATA_OK)
      return err;

    // Write one sector
    outsw(ATA_PRIMARY_BASE + ATA_REG_DATA, buf + (i * ATA_SECTOR_SIZE), 256);

    // Flush the drive's write cache
    outb(ATA_PRIMARY_BASE + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    err = ata_poll(0);
    if (err != ATA_OK)
      return err;
  }

  return ATA_OK;
}
