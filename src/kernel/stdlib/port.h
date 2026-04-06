#ifndef PORT_H
#define PORT_H

#include "stdint.h"

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void insw(uint16_t port, void *buf, uint32_t count);
void outsw(uint16_t port, const void *buf, uint32_t count);

#endif
