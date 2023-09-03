#include "print.h"

void kernel_main()
{
    print_clear();
    print_set_color(PRINT_COLOR_CYAN, PRINT_COLOR_BLACK);
    for (int i = 0; i < 2; i++)
    {
        print_line("Hello 64-bit Kernel from C!");
    }
}