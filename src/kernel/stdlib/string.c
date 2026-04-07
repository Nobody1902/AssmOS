#include "string.h"
#include <stddef.h>
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

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (n--) {
    if (*p1 != *p2)
      return (*p1 > *p2) ? 1 : -1;
    ++p1;
    ++p2;
  }
  return 0;
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

const char *strchr(const char *s, int c) {
  unsigned char uc = (unsigned char)c;
  for (;; ++s) {
    if ((unsigned char)*s == uc)
      return s;
    if (*s == '\0')
      break;
  }
  return NULL;
}

char *strtok(char *s, const char *delim) {
  static char *next = NULL;
  char *start;
  if (s != NULL)
    next = s;
  if (next == NULL)
    return NULL;

  for (; *next != '\0'; ++next) {
    const char *d = delim;
    int is_delim = 0;
    while (*d != '\0') {
      if (*next == *d) {
        is_delim = 1;
        break;
      }
      ++d;
    }
    if (!is_delim)
      break;
  }

  if (*next == '\0') {
    next = NULL;
    return NULL;
  }

  start = next;

  for (; *next != '\0'; ++next) {
    const char *d = delim;
    int is_delim = 0;
    while (*d != '\0') {
      if (*next == *d) {
        is_delim = 1;
        break;
      }
      ++d;
    }
    if (is_delim) {
      *next = '\0';
      ++next;
      return start;
    }
  }

  next = NULL;
  return start;
}
