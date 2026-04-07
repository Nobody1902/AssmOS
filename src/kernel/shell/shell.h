#ifndef SHELL_H
#define SHELL_H

#include "../stdlib/disk/fat32/fat32.h"

#define MAX_CMD_LEN 512

int shell(fat32_t *fs);

#endif
