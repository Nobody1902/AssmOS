#include "port.h"

void outb(uint16_t port, uint8_t value) {
  __asm__ volatile("movw %[port], %%dx;"
                   "movb %[value], %%al;"
                   "outb %%al, %%dx;"
                   :
                   : [port] "r"(port), [value] "r"(value)
                   : "%dx", "%al");
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("in %%dx, %%al" : "=a"(ret) : "d"(port) : "memory");
  return ret;
}
