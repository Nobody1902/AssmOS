#include "fat32.h"
#include "../../mem/heap.h"
#include "../../stdio.h"
#include "../../string.h"

#define FAT32_EOC 0x0FFFFFF8

// ─────────────────────────────
// Low-level helpers
// ─────────────────────────────

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

static int set_cluster(fat32_t *fs, uint32_t cluster, uint32_t val) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  uint32_t offset = cluster * 4;
  uint32_t sector = fs->fat_start_lba + (offset / 512);

  if (fs->dev->read(fs->dev, sector, 1, buf)) {
    kfree(buf);
    return -1;
  }

  buf[offset % 512] = val & 0xFF;
  buf[offset % 512 + 1] = (val >> 8) & 0xFF;
  buf[offset % 512 + 2] = (val >> 16) & 0xFF;
  buf[offset % 512 + 3] = (val >> 24) & 0xFF;

  int res = fs->dev->write(fs->dev, sector, 1, buf);
  kfree(buf);
  return res;
}

static uint32_t alloc_cluster(fat32_t *fs) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return 0;

  for (uint32_t i = 2; i < fs->bpb.total_sectors_32; i++) {
    uint32_t off = i * 4;
    uint32_t sec = fs->fat_start_lba + (off / 512);

    if (fs->dev->read(fs->dev, sec, 1, buf)) {
      kfree(buf);
      return 0;
    }

    uint32_t val = buf[off % 512] | (buf[off % 512 + 1] << 8) |
                   (buf[off % 512 + 2] << 16) | (buf[off % 512 + 3] << 24);

    if ((val & 0x0FFFFFFF) == 0) {
      val = 0x0FFFFFFF;
      buf[off % 512] = val & 0xFF;
      buf[off % 512 + 1] = (val >> 8) & 0xFF;
      buf[off % 512 + 2] = (val >> 16) & 0xFF;
      buf[off % 512 + 3] = (val >> 24) & 0xFF;
      fs->dev->write(fs->dev, sec, 1, buf);
      kfree(buf);
      return i;
    }
  }

  kfree(buf);
  return 0;
}

static void free_chain(fat32_t *fs, uint32_t cluster) {
  while (cluster < FAT32_EOC) {
    uint32_t next = next_cluster(fs, cluster);
    set_cluster(fs, cluster, 0);
    cluster = next;
  }
}

static uint32_t entry_cluster(fat32_dir_entry_t *e) {
  return ((uint32_t)e->first_cluster_high << 16) | e->first_cluster_low;
}

// ─────────────────────────────
// Helpers for filenames
// ─────────────────────────────
static void to_83(const char *in, char out[11]) {
  for (int i = 0; i < 11; i++)
    out[i] = ' ';

  int i = 0, j = 0;
  while (in[i] && in[i] != '.' && j < 8) {
    char c = in[i++];
    if (c >= 'a' && c <= 'z')
      c -= 32;
    out[j++] = c;
  }

  if (in[i] == '.') {
    i++;
    j = 8;
    for (int k = 0; k < 3; k++) {
      if (in[i]) {
        char c = in[i++];
        if (c >= 'a' && c <= 'z')
          c -= 32;
        out[j++] = c;
      } else
        out[j++] = ' ';
    }
  }
}

// ─────────────────────────────
// Directory helpers
// ─────────────────────────────
static int find_entry(fat32_t *fs, uint32_t dir, const char *name,
                      fat32_dir_entry_t *out, uint32_t *out_lba, int *out_idx) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  char name83[11];
  to_83(name, name83);

  while (dir < FAT32_EOC) {
    uint32_t lba = cluster_to_lba(fs, dir);

    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, buf)) {
        kfree(buf);
        return -1;
      }

      fat32_dir_entry_t *e = (void *)buf;
      for (int i = 0; i < 16; i++) {
        if (e[i].name[0] == 0x00) {
          kfree(buf);
          return -1;
        }
        if ((uint8_t)e[i].name[0] == 0xE5)
          continue;
        if (e[i].attr == 0x0F)
          continue;

        if (memcmp(e[i].name, name83, 11) == 0) {
          if (out)
            *out = e[i];
          if (out_lba)
            *out_lba = lba + s;
          if (out_idx)
            *out_idx = i;
          kfree(buf);
          return 0;
        }
      }
    }

    dir = next_cluster(fs, dir);
  }

  kfree(buf);
  return -1;
}

static int insert_entry(fat32_t *fs, uint32_t dir, const char *name,
                        fat32_dir_entry_t *entry_template) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  char name83[11];
  to_83(name, name83);

  while (dir < FAT32_EOC) {
    uint32_t lba = cluster_to_lba(fs, dir);

    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, buf)) {
        kfree(buf);
        return -1;
      }

      fat32_dir_entry_t *e = (void *)buf;
      for (int i = 0; i < 16; i++) {
        if (e[i].name[0] == 0x00 || (uint8_t)e[i].name[0] == 0xE5) {
          // Fill in entry
          memcpy(e[i].name, name83, 11);
          if (entry_template)
            memcpy(&e[i], entry_template, sizeof(fat32_dir_entry_t));
          fs->dev->write(fs->dev, lba + s, 1, buf);
          kfree(buf);
          return 0;
        }
      }
    }

    // Need next cluster?
    uint32_t next = next_cluster(fs, dir);
    if (next >= FAT32_EOC) {
      // Allocate new cluster for directory
      next = alloc_cluster(fs);
      if (!next) {
        kfree(buf);
        return -1;
      }
      set_cluster(fs, dir, next);

      // Zero out new cluster
      uint32_t new_lba = cluster_to_lba(fs, next);
      memset(buf, 0, 512);
      for (uint32_t i = 0; i < fs->sectors_per_cluster; i++)
        fs->dev->write(fs->dev, new_lba + i, 1, buf);
    }

    dir = next;
  }

  kfree(buf);
  return -1;
}

// ─────────────────────────────
// FAT32 mounting and listing
// ─────────────────────────────

int fat32_mount(fat32_t *fs, block_device_t *dev) {
  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  if (dev->read(dev, 0, 1, buf) != 0) {
    kfree(buf);
    return -1;
  }

  fs->dev = dev;
  fs->bpb = *(fat32_bpb_t *)buf;
  fs->fat_start_lba = fs->bpb.reserved_sector_count;
  fs->data_start_lba =
      fs->bpb.reserved_sector_count + (fs->bpb.num_fats * fs->bpb.fat_size_32);
  fs->sectors_per_cluster = fs->bpb.sectors_per_cluster;

  kfree(buf);
  return 0;
}

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

      fat32_dir_entry_t *e = (void *)buf;
      for (int i = 0; i < 16; i++) {
        if (e[i].name[0] == 0x00) {
          kfree(buf);
          return 0;
        }
        if ((uint8_t)e[i].name[0] == 0xE5)
          continue;
        if (e[i].attr == 0x0F)
          continue;

        char name[12];
        memcpy(name, e[i].name, 11);
        name[11] = 0;
        printf("%s\n", name);
      }
    }

    cluster = next_cluster(fs, cluster);
  }

  kfree(buf);
  return 0;
}

int fat32_file_size(fat32_t *fs, const char *path, uint32_t *size) {
  fat32_dir_entry_t *e = kmalloc(sizeof(fat32_dir_entry_t));
  if (find_entry(fs, fs->bpb.root_cluster, path, e, NULL, NULL)) {
    return -1; // file not found
  }

  if (size)
    *size = e->file_size;

  kfree(e);

  return 0;
}

// ─────────────────────────────
// FAT32 file read/write
// ─────────────────────────────

int fat32_read(fat32_t *fs, const char *path, void *buf, uint32_t size) {
  fat32_dir_entry_t e;
  if (find_entry(fs, fs->bpb.root_cluster, path, &e, NULL, NULL))
    return -1;

  uint32_t cluster = entry_cluster(&e);
  uint32_t read = 0;
  uint8_t *tmp = kmalloc(512);
  if (!tmp)
    return -1;

  while (cluster < FAT32_EOC && read < size) {
    uint32_t lba = cluster_to_lba(fs, cluster);
    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      if (fs->dev->read(fs->dev, lba + s, 1, tmp)) {
        kfree(tmp);
        return -1;
      }

      uint32_t to_copy = 512;
      if (read + to_copy > size)
        to_copy = size - read;

      memcpy((uint8_t *)buf + read, tmp, to_copy);
      read += to_copy;
      if (read >= size)
        break;
    }
    cluster = next_cluster(fs, cluster);
  }

  kfree(tmp);
  return read;
}

int fat32_write(fat32_t *fs, const char *path, const void *buf, uint32_t size) {
  // Allocate first cluster for file
  uint32_t first = alloc_cluster(fs);
  if (!first)
    return -1;

  uint32_t current = first;
  uint32_t written = 0;
  uint8_t *tmp = kmalloc(512);
  if (!tmp)
    return -1;

  while (written < size) {
    uint32_t lba = cluster_to_lba(fs, current);
    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
      memset(tmp, 0, 512);

      uint32_t to_copy = 512;
      if (written + to_copy > size)
        to_copy = size - written;

      memcpy(tmp, (uint8_t *)buf + written, to_copy);
      fs->dev->write(fs->dev, lba + s, 1, tmp);

      written += to_copy;
      if (written >= size)
        break;
    }

    if (written < size) {
      uint32_t next = alloc_cluster(fs);
      if (!next) {
        kfree(tmp);
        return -1;
      }
      set_cluster(fs, current, next);
      current = next;
    }
  }

  set_cluster(fs, current, FAT32_EOC);
  kfree(tmp);

  // Prepare directory entry
  fat32_dir_entry_t entry = {0};
  entry.first_cluster_low = first & 0xFFFF;
  entry.first_cluster_high = (first >> 16) & 0xFFFF;
  entry.attr = 0x20; // archive/file

  insert_entry(fs, fs->bpb.root_cluster, path, &entry);

  return written;
}

// ─────────────────────────────
// FAT32 mkdir / rm / rmdir
// ─────────────────────────────

int fat32_mkdir(fat32_t *fs, const char *path) {
  uint32_t cl = alloc_cluster(fs);
  if (!cl)
    return -1;

  uint8_t *zero = kmalloc(512);
  if (!zero)
    return -1;
  memset(zero, 0, 512);

  uint32_t lba = cluster_to_lba(fs, cl);
  for (uint32_t i = 0; i < fs->sectors_per_cluster; i++)
    fs->dev->write(fs->dev, lba + i, 1, zero);
  kfree(zero);

  // Prepare directory entry
  fat32_dir_entry_t entry = {0};
  entry.first_cluster_low = cl & 0xFFFF;
  entry.first_cluster_high = (cl >> 16) & 0xFFFF;
  entry.attr = 0x10; // directory

  insert_entry(fs, fs->bpb.root_cluster, path, &entry);
  return 0;
}

int fat32_rm(fat32_t *fs, const char *path) {
  fat32_dir_entry_t e;
  uint32_t lba;
  int idx;

  if (find_entry(fs, fs->bpb.root_cluster, path, &e, &lba, &idx))
    return -1;

  free_chain(fs, entry_cluster(&e));

  uint8_t *buf = kmalloc(512);
  if (!buf)
    return -1;

  if (fs->dev->read(fs->dev, lba, 1, buf) == 0) {
    ((fat32_dir_entry_t *)buf)[idx].name[0] = 0xE5;
    fs->dev->write(fs->dev, lba, 1, buf);
  }

  kfree(buf);
  return 0;
}

int fat32_rmdir(fat32_t *fs, const char *path) { return fat32_rm(fs, path); }
