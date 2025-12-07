#include "string.h"
#include <stdint.h>

void *memset(void *p, int c, size_t count) {
  for (size_t i = 0; i < count; i++) {
    ((int *)p)[i] = c;
  }
  return p;
}

void *memcpy(void *dest, const void *src, size_t len) {
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

size_t strlen(const char *str) {
  size_t ret = 0;
  while (str[ret] != '\0')
    ret++;
  return ret;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 == *s2; *s2++, *s1++)
    if (*s1 == '\0')
      return 0;
  return *s1 - *s2;
}
