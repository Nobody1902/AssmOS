#ifndef INT_H
#define INT_H

#include <stdint.h>

int itoa(int32_t value, char *result, int base);
int utoa(uint32_t value, char *result, int base);

#endif
