#ifndef FAT32_H
#define FAT32_H

#include "../block_device.h"
#include <stdint.h>

typedef struct {
  uint8_t jmp[3];
  uint8_t oem[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sector_count;
  uint8_t num_fats;
  uint16_t root_entry_count;
  uint16_t total_sectors_16;
  uint8_t media;
  uint16_t fat_size_16;
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;
  uint32_t fat_size_32;
  uint16_t ext_flags;
  uint16_t fs_version;
  uint32_t root_cluster;
} __attribute__((packed)) fat32_bpb_t;

typedef struct {
  char name[11];
  uint8_t attr;
  uint8_t nt_res;
  uint8_t crt_time_tenth;
  uint16_t crt_time;
  uint16_t crt_date;
  uint16_t lst_acc_date;
  uint16_t first_cluster_high;
  uint16_t wrt_time;
  uint16_t wrt_date;
  uint16_t first_cluster_low;
  uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

typedef struct {
  block_device_t *dev;
  fat32_bpb_t bpb;
  uint32_t fat_start_lba;
  uint32_t data_start_lba;
  uint8_t sectors_per_cluster;
} fat32_t;

// Minimal FAT32 functions
int fat32_mount(fat32_t *fs, block_device_t *dev);
int fat32_listdir(fat32_t *fs, const char *path);
int fat32_file_size(fat32_t *fs, const char *path, uint32_t *size);
int fat32_read(fat32_t *fs, const char *path, void *buf, uint32_t size);

#endif
