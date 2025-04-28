#include "int.h"

char *itoa(int value, char *result, int base) {
  if (base < 2 || base > 36) {
    *result = '\0';
    return result;
  }

  unsigned int val;
  char *ptr = result;

  if (value < 0) {
    *ptr++ = '-';
    // Handle INT_MIN carefully
    val = (unsigned int)(-(value + 1)) + 1;
  } else {
    val = (unsigned int)value;
  }

  char *digits_start = utoa(val, ptr, base);

  return result;
}

char *utoa(unsigned int value, char *result, int base) {
  if (base < 2 || base > 36) {
    *result = '\0';
    return result;
  }

  char *ptr = result;
  char *ptr1;
  char tmp_char;
  const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

  // Handle zero explicitly
  if (value == 0) {
    *ptr++ = '0';
    *ptr = '\0';
    return result;
  }

  // Convert number to string in reverse order
  while (value != 0) {
    unsigned int remainder = value % base;
    value /= base;
    *ptr++ = digits[remainder];
  }

  *ptr = '\0';

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

  return result;
}
