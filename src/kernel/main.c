#include "shell/shell.h"
#include "stdlib/float.h"
#include "stdlib/keyboard/input.h"
#include "stdlib/stdio.h"
#include "stdlib/vga/print.h"

void __stack_chk_fail() { print_line("ERROR: __stack_chk_fail\n"); }

void kernel_main() {
  printf("Hello World!\n");
  shell();
}
