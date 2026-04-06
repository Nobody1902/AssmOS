#include "port.h"

inline void outb(uint16_t port, uint8_t value) {
  __asm__ volatile("movw %[port], %%dx;"
                   "movb %[value], %%al;"
                   "outb %%al, %%dx;"
                   :
                   : [port] "r"(port), [value] "r"(value)
                   : "%dx", "%al");
}

inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("in %%dx, %%al" : "=a"(ret) : "d"(port) : "memory");
  return ret;
}

inline void insw(uint16_t port, void *buf, uint32_t count) {
  __asm__ volatile("rep insw" : "+D"(buf), "+c"(count) : "d"(port) : "memory");
}

inline void outsw(uint16_t port, const void *buf, uint32_t count) {
  __asm__ volatile("rep outsw" : "+S"(buf), "+c"(count) : "d"(port) : "memory");
}
