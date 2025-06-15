#include "shutdown.h"
#include "port.h"

void shutdown() {
  asm volatile("mov $0x5301, %%ax\n\t"
               "xor %%bx, %%bx\n\t"
               "int $0x15\n\t"
               :
               :
               : "ax", "bx");

  for (;;) {
    asm volatile("hlt");
  }
}

void reboot() {
  asm volatile("cli");
  uint8_t good = 0x02;
  while (good & 0x02)
    good = inb(0x64);
  outb(0x64, 0xFE);

  for (;;) {
    asm volatile("hlt");
  }
}
