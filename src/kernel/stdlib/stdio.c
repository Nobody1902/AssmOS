#include "stdio.h"
#include "float.h"
#include "int.h"
#include "vga/print.h"
#include <stdarg.h>
#include <stdbool.h>

void putc(char c) { print_char(c); }

void puts(char *s) {
  while (*s)
    putc(*s++);
}

void printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char numbuf[PRINTF_NUM_BUF];

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
      case 'd':
      case 'i':
        itoa(va_arg(args, int), numbuf, 10);
        puts(numbuf);
        break;
      case 'u':
        utoa(va_arg(args, unsigned int), numbuf, 10);
        puts(numbuf);
        break;
      case 'x':
        utoa(va_arg(args, unsigned int), numbuf, 16);
        puts(numbuf);
        break;
      case 'X':
        utoa(va_arg(args, unsigned int), numbuf, 16);
        for (char *p = numbuf; *p; p++)
          if (*p >= 'a' && *p <= 'f')
            *p -= 32;
        puts(numbuf);
        break;
      case 'o':
        utoa(va_arg(args, unsigned int), numbuf, 8);
        puts(numbuf);
        break;
      case 'f':
        ftos((float)va_arg(args, double), numbuf);
        puts(numbuf);
        break;
      case 's': {
        char *s = va_arg(args, char *);
        if (!s)
          s = "(null)";
        puts(s);
        break;
      }
      case 'c':
        putc((char)va_arg(args, int));
        break;
      case '%':
        putc('%');
        break;
      default:
        putc('%');
        putc(*fmt);
        break;
      }
    } else {
      putc(*fmt);
    }
    fmt++;
  }

  va_end(args);
}
