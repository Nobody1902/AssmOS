#include "keyboard/input.h"
#include "stdio.h"
#include "vga/print.h"

void __stack_chk_fail() { print_line("ERROR: __stack_chk_fail"); }

void kernel_main() {
  printf("Hello World!\n");
  char buff[255];
  scanf("%s", buff);

  print_clear();

  printf("%s\n", buff);
}
