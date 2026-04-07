#include "fat32.h"
#include "../../mem/heap.h"
#include "../../stdio.h"
#include "../../string.h"

#define FAT32_EOC 0x0FFFFFF8

static uint32_t cluster_to_lba(fat32_t *fs, uint32_t cluster) {
  return fs->data_start_lba + (cluster - 2) * fs->sectors_per_cluster;
}

static uint32_t next_cluster(fat32_t *fs, uint32_t cluster) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return FAT32_EOC;

  uint32_t offset = cluster * 4;
  uint32_t sector = fs->fat_start_lba + (offset / 512);

  if (fs->dev->read(fs->dev, sector, 1, buf)) {
    kfree(buf);
    return FAT32_EOC;
  }

  uint32_t val = buf[offset % 512] | (buf[offset % 512 + 1] << 8) |
                 (buf[offset % 512 + 2] << 16) | (buf[offset % 512 + 3] << 24);
  kfree(buf);

  return val & 0x0FFFFFFF;
}

int fat32_mount(fat32_t *fs, block_device_t *dev) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  if (dev->read(dev, 0, 1, buf)) {
    kfree(buf);
    return -1;
  }

  fs->dev = dev;
  fs->bpb = *(fat32_bpb_t *)buf;
  fs->fat_start_lba = fs->bpb.reserved_sector_count;
  fs->data_start_lba =
      fs->bpb.reserved_sector_count + fs->bpb.num_fats * fs->bpb.fat_size_32;
  fs->sectors_per_cluster = fs->bpb.sectors_per_cluster;

  kfree(buf);
  return 0;
}

// List directory entries at root
int fat32_listdir(fat32_t *fs, const char *path) {
  uint32_t cluster = fs->bpb.root_cluster;
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  while (cluster < FAT32_EOC) {
    uint32_t lba = cluster_to_lba(fs, cluster);

    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, buf)) {
        kfree(buf);
        return -1;
      }

      fat32_dir_entry_t *entries = (fat32_dir_entry_t *)buf;
      for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00) {
          kfree(buf);
          return 0;
        }
        if ((uint8_t)entries[i].name[0] == 0xE5)
          continue;
        if (entries[i].attr == 0x0F)
          continue;

        char name[12];
        memcpy(name, entries[i].name, 11);
        name[11] = 0;
        printf("%s\n", name);
      }
    }
    cluster = next_cluster(fs, cluster);
  }

  kfree(buf);
  return 0;
}

// Get filesize by filename
int fat32_file_size(fat32_t *fs, const char *filename, uint32_t *size) {
  uint32_t cluster = fs->bpb.root_cluster;
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  while (cluster < FAT32_EOC) {
    uint32_t lba = cluster_to_lba(fs, cluster);
    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, buf)) {
        kfree(buf);
        return -1;
      }

      fat32_dir_entry_t *entries = (fat32_dir_entry_t *)buf;
      for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00) {
          kfree(buf);
          return -1;
        }
        if ((uint8_t)entries[i].name[0] == 0xE5)
          continue;
        if (entries[i].attr == 0x0F)
          continue;

        char name83[12];
        memcpy(name83, entries[i].name, 11);
        name83[11] = 0;

        if (strncmp(name83, filename, 11) == 0) {
          if (size)
            *size = entries[i].file_size;
          kfree(buf);
          return 0;
        }
      }
    }
    cluster = next_cluster(fs, cluster);
  }

  kfree(buf);
  return -1;
}

// Read file into buffer
int fat32_read(fat32_t *fs, const char *filename, void *dest, uint32_t size) {
  uint32_t cluster = fs->bpb.root_cluster;
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  // Find file entry
  fat32_dir_entry_t file_entry;
  int found = 0;
  while (cluster < FAT32_EOC && !found) {
    uint32_t lba = cluster_to_lba(fs, cluster);
    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, buf)) {
        kfree(buf);
        return -1;
      }

      fat32_dir_entry_t *entries = (fat32_dir_entry_t *)buf;
      for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00)
          break;
        if ((uint8_t)entries[i].name[0] == 0xE5)
          continue;
        if (entries[i].attr == 0x0F)
          continue;

        char name83[12];
        memcpy(name83, entries[i].name, 11);
        name83[11] = 0;

        if (strncmp(name83, filename, 11) == 0) {
          file_entry = entries[i];
          found = 1;
          break;
        }
      }
    }
    if (!found)
      cluster = next_cluster(fs, cluster);
  }

  if (!found) {
    kfree(buf);
    return -1;
  }

  // Read file clusters
  uint32_t file_cluster = ((uint32_t)file_entry.first_cluster_high << 16) |
                          file_entry.first_cluster_low;
  uint32_t read_bytes = 0;

  while (file_cluster < FAT32_EOC && read_bytes < size) {
    uint32_t lba = cluster_to_lba(fs, file_cluster);

    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, buf)) {
        kfree(buf);
        return -1;
      }

      uint32_t to_copy = 512;
      if (read_bytes + to_copy > size)
        to_copy = size - read_bytes;

      memcpy((uint8_t *)dest + read_bytes, buf, to_copy);
      read_bytes += to_copy;
      if (read_bytes >= size)
        break;
    }
    file_cluster = next_cluster(fs, file_cluster);
  }

  kfree(buf);
  return read_bytes;
}
