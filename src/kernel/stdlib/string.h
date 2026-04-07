typedef unsigned long size_t;

#ifndef STRING_H
#define STRING_H

void *memcpy(void *dest, const void *src, size_t len);
void *memset(void *p, int c, size_t count);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
const char *strchr(const char *s, int c);
char *strtok(char *s, const char *delim);
size_t strnlen(const char *s, size_t maxlen);
int strncmp(const char *s1, const char *s2, size_t n);

#endif
