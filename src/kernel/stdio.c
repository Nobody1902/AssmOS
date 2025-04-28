#include "stdio.h"
#include "keyboard/input.h"
#include "stdlib/int.h"
#include "vga/print.h"
#include <stdarg.h>
#include <stdbool.h>

void putc(char c) { print_char(c); }
char inc() {
  bool shiftPressed = false;
  while (true) {
    keyCode_t key = scanKey();

    if (!isAscii(key)) {
      if (key == LEFT_SHIFT_PRESSED) {
        shiftPressed = true;
      } else if (key == LEFT_SHIFT_RELEASED) {
        shiftPressed = false;
      }

      continue;
    }
    return toAscii(key, shiftPressed);
  }
}

void puts(char *s) {
  while (*s) {
    putc(*s);
    s++;
  }
}

void printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == 'd' || *fmt == 'i') {
        char *buff = itoa(va_arg(args, int), buff, 10);
        puts(buff);
      } else if (*fmt == 'u') {
        char *buff = utoa(va_arg(args, unsigned int), buff, 10);
        puts(buff);
      } else if (*fmt == 'f' || *fmt == 'F') {
        puts("(%f,%F aren't yet implemented)"); // TODO: Implement floats
      } else if (*fmt == 'x' || *fmt == 'X') {
        char *buff = utoa(va_arg(args, unsigned int), buff, 16);
        puts(buff);
      } else if (*fmt == 'o') {
        char *buff = utoa(va_arg(args, unsigned int), buff, 8);
        puts(buff);
      } else if (*fmt == 's') {
        puts(va_arg(args, char *));
      } else if (*fmt == 'c') {
        putc((char)va_arg(args, int));
      } else if (*fmt == '%') {
        putc('%');
      }
    } else {
      putc(*fmt);
    }
    fmt++;
  }
  va_end(args);
}

int strtoi_base(const char *str, int base, int *out_value) {
  int value = 0;
  bool negative = false;
  const char *p = str;

  // Skip whitespace
  while (*p == ' ' || *p == '\t' || *p == '\n')
    p++;

  // Optional sign for base 10 and 16
  if (base == 10 || base == 16) {
    if (*p == '-') {
      negative = true;
      p++;
    } else if (*p == '+') {
      p++;
    }
  }

  if (base == 16) {
    // Optional "0x" or "0X" prefix
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
      p += 2;
    }
  } else if (base == 8) {
    // Optional "0" prefix
    if (*p == '0') {
      p++;
    }
  }

  bool digit_found = false;
  while (*p) {
    char c = *p;
    int digit = -1;

    if (c >= '0' && c <= '9')
      digit = c - '0';
    else if (c >= 'a' && c <= 'f')
      digit = 10 + (c - 'a');
    else if (c >= 'A' && c <= 'F')
      digit = 10 + (c - 'A');
    else
      break;

    if (digit >= base)
      break;

    value = value * base + digit;
    digit_found = true;
    p++;
  }

  if (!digit_found)
    return 0; // No digits parsed

  *out_value = negative ? -value : value;
  return 1; // success
}

void read_string(char *buf, int max_len, bool ignore_whitespace) {
  int i = 0;
  char c;

  do {
    c = inc();
  } while (c == ' ' || c == '\t' || c == '\n');

  // Read until whitespace or max_len-1
  while ((ignore_whitespace || (c != ' ' && c != '\t')) && c != '\n' &&
         c != '\0' && i < max_len - 1) {
    buf[i++] = c;
    c = inc();
  }
  buf[i] = '\0';
}

int scanf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int assigned = 0;
  char input_buf[64]; // buffer for reading strings and numbers

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == 'd' || *fmt == 'i') {
        // Read string, convert base 10 signed int
        read_string(input_buf, sizeof(input_buf), false);
        int val;
        if (strtoi_base(input_buf, 10, &val)) {
          int *p = va_arg(args, int *);
          *p = val;
          assigned++;
        }
      } else if (*fmt == 'u') {
        // Read string, convert base 10 unsigned int
        read_string(input_buf, sizeof(input_buf), false);
        int val;
        if (strtoi_base(input_buf, 10, &val)) {
          unsigned int *p = va_arg(args, unsigned int *);
          *p = (unsigned int)val;
          assigned++;
        }
      } else if (*fmt == 'x' || *fmt == 'X') {
        // Read string, convert base 16 unsigned int
        read_string(input_buf, sizeof(input_buf), false);
        int val;
        if (strtoi_base(input_buf, 16, &val)) {
          unsigned int *p = va_arg(args, unsigned int *);
          *p = (unsigned int)val;
          assigned++;
        }
      } else if (*fmt == 'o') {
        // Read string, convert base 8 unsigned int
        read_string(input_buf, sizeof(input_buf), false);
        int val;
        if (strtoi_base(input_buf, 8, &val)) {
          unsigned int *p = va_arg(args, unsigned int *);
          *p = (unsigned int)val;
          assigned++;
        }
      } else if (*fmt == 's') {
        // Read string until whitespace
        char *p = va_arg(args, char *);
        read_string(p, 256, true); // assume caller allocated enough space
        assigned++;
      } else if (*fmt == 'c') {
        // Read single char
        char *p = va_arg(args, char *);
        *p = inc();
        assigned++;
      } else if (*fmt == '%') {
        // Literal '%', consume one char from input and compare
        char c = inc();
        if (c != '%') {
          // mismatch, stop parsing
          break;
        }
      } else {
        // Unsupported format specifier, ignore or break
        break;
      }
      fmt++;
    } else {
      // Match literal character from input
      char c = inc();
      if (c != *fmt) {
        break;
      }
      fmt++;
    }
  }

  va_end(args);
  return assigned;
}
