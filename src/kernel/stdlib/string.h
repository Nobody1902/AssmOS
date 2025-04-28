#include <stdio.h>

#ifndef STRING_H
#define STRING_H

void *memset(void *p, int c, size_t count);
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);

#endif
