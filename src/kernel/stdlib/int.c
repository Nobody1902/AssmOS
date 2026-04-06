#include "int.h"

int itoa(int32_t value, char *result, int base) {
  int len = 0;
  if (base < 2 || base > 36) {
    *result = '\0';
    return len;
  }

  unsigned int val;
  char *ptr = result;

  if (value < 0) {
    ptr[len++] = '-';
    // Handle INT_MIN carefully
    val = (unsigned int)(-(value + 1)) + 1;
  } else {
    val = (unsigned int)value;
  }

  len += utoa(val, ptr, base);

  return len;
}

int utoa(uint32_t value, char *result, int base) {
  int len = 0;
  if (base < 2 || base > 36) {
    *result = '\0';
    return -1;
  }

  char *ptr = result;
  char *ptr1;
  char tmp_char;
  const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

  // Handle zero explicitly
  if (value == 0) {
    ptr[len++] = '0';
    ptr[len] = '\0';
    return len;
  }

  // Convert number to string in reverse order
  while (value != 0) {
    unsigned int remainder = value % base;
    value /= base;
    ptr[len++] = digits[remainder];
  }

  ptr[len] = '\0';

  // Reverse the string
  ptr1 = result;
  ptr--;
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr = *ptr1;
    *ptr1 = tmp_char;
    ptr--;
    ptr1++;
  }

  return len;
}
