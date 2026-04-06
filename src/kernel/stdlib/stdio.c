#include "stdio.h"
#include "float.h"
#include "int.h"
#include "keyboard/input.h"
#include "mem/heap.h"
#include "vga/print.h"
#include <stdarg.h>
#include <stdbool.h>

void putc(char c) { print_char(c); }

char inc() {
  bool shiftPressed = false;
  while (true) {
    keyCode_t key = scanKey();
    if (!isAscii(key)) {
      if (key == LEFT_SHIFT_PRESSED)
        shiftPressed = true;
      else if (key == LEFT_SHIFT_RELEASED)
        shiftPressed = false;
      continue;
    }
    return toAscii(key, shiftPressed);
  }
}

void puts(char *s) {
  while (*s)
    putc(*s++);
}

void printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      char *buff = NULL;

      switch (*fmt) {
      case 'd':
      case 'i':
        buff = (char *)kmalloc(12);
        if (!buff)
          return;
        itoa(va_arg(args, int), buff, 10);
        puts(buff);
        kfree(buff);
        break;

      case 'u':
        buff = (char *)kmalloc(11);
        if (!buff)
          return;
        utoa(va_arg(args, unsigned int), buff, 10);
        puts(buff);
        kfree(buff);
        break;

      case 'x':
      case 'X':
        buff = (char *)kmalloc(9);
        if (!buff)
          return;
        utoa(va_arg(args, unsigned int), buff, 16);
        if (*fmt == 'X') {
          for (char *p = buff; *p; p++)
            if (*p >= 'a' && *p <= 'f')
              *p -= 32;
        }
        puts(buff);
        kfree(buff);
        break;

      case 'o':
        buff = (char *)kmalloc(12);
        if (!buff)
          return;
        utoa(va_arg(args, unsigned int), buff, 8);
        puts(buff);
        kfree(buff);
        break;

      case 'f':
        buff = (char *)kmalloc(64);
        if (!buff)
          return;
        ftos((float)va_arg(args, double), buff);
        puts(buff);
        kfree(buff);
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
int strtoi_base(const char *str, int base, int *out_value) {
  int value = 0;
  bool negative = false;
  const char *p = str;

  while (*p == ' ' || *p == '\t' || *p == '\n')
    p++;

  if (base == 10 || base == 16) {
    if (*p == '-') {
      negative = true;
      p++;
    } else if (*p == '+')
      p++;
  }

  if (base == 16 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    p += 2;
  else if (base == 8 && *p == '0')
    p++;

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
    return 0;

  *out_value = negative ? -value : value;
  return 1;
}

static char *read_string_dynamic(bool ignore_whitespace) {
  int capacity = 16;
  int len = 0;
  char *buf = (char *)kmalloc(capacity);
  if (!buf)
    return NULL;

  char c;
  do {
    c = inc();
  } while (c == ' ' || c == '\t' || c == '\n');

  while (ignore_whitespace || (c != ' ' && c != '\t')) {
    if (c == '\n' || c == '\0')
      break;

    if (len + 1 >= capacity) {
      int new_cap = capacity * 2;
      char *new_buf = (char *)kmalloc(new_cap);
      if (!new_buf) {
        kfree(buf);
        return NULL;
      }
      for (int i = 0; i < len; i++)
        new_buf[i] = buf[i];
      kfree(buf);
      buf = new_buf;
      capacity = new_cap;
    }

    buf[len++] = c;
    c = inc();
  }

  buf[len] = '\0';
  return buf;
}

int scanf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int assigned = 0;

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      char *input_buf = NULL;

      if (*fmt == 'd' || *fmt == 'i') {
        input_buf = read_string_dynamic(false);
        if (!input_buf)
          break;

        int val;
        if (strtoi_base(input_buf, 10, &val)) {
          int *p = va_arg(args, int *);
          *p = val;
          assigned++;
        }
        kfree(input_buf);

      } else if (*fmt == 'u') {
        input_buf = read_string_dynamic(false);
        if (!input_buf)
          break;

        int val;
        if (strtoi_base(input_buf, 10, &val)) {
          unsigned int *p = va_arg(args, unsigned int *);
          *p = (unsigned int)val;
          assigned++;
        }
        kfree(input_buf);

      } else if (*fmt == 'x' || *fmt == 'X') {
        input_buf = read_string_dynamic(false);
        if (!input_buf)
          break;

        int val;
        if (strtoi_base(input_buf, 16, &val)) {
          unsigned int *p = va_arg(args, unsigned int *);
          *p = (unsigned int)val;
          assigned++;
        }
        kfree(input_buf);

      } else if (*fmt == 'o') {
        input_buf = read_string_dynamic(false);
        if (!input_buf)
          break;

        int val;
        if (strtoi_base(input_buf, 8, &val)) {
          unsigned int *p = va_arg(args, unsigned int *);
          *p = (unsigned int)val;
          assigned++;
        }
        kfree(input_buf);

      } else if (*fmt == 's') {
        char **p = va_arg(args, char **);
        char *str = read_string_dynamic(true);
        if (!str)
          break;
        *p = str;
        assigned++;

      } else if (*fmt == 'c') {
        char *p = va_arg(args, char *);
        *p = inc();
        assigned++;

      } else if (*fmt == '%') {
        char c = inc();
        if (c != '%')
          break;

      } else
        break;

      fmt++;
    } else {
      char c = inc();
      if (c != *fmt)
        break;
      fmt++;
    }
  }

  va_end(args);
  return assigned;
}
