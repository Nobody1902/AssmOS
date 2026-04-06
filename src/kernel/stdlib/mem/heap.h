#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

extern char __heap_start[];
extern char __heap_end[];
#define PAGE_SIZE 4096

void heap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
size_t kalloc_usable_size(void *ptr);

#endif
