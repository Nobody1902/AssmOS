#include "float.h"
#include "int.h"
#include <stdint.h>

#define DECIMAL_PLACES 7
#define SCALE (10000000U) // 10^DECIMAL_PLACES

int ftos(float input, char *buf) {
  union {
    float f;
    uint32_t u;
  } u;
  u.f = input;

  uint32_t bits = u.u;
  uint32_t sign = bits >> 31;
  int32_t exponent = ((bits >> 23) & 0xFF) - 127;
  uint32_t mantissa = bits & 0x7FFFFF;

  char *start = buf;

  // Handle zero
  if ((bits & 0x7FFFFFFF) == 0) {
    if (sign)
      *buf++ = '-';
    *buf++ = '0';
    *buf++ = '.';
    *buf++ = '0';
    *buf = '\0';
    return buf - start;
  }

  // Handle Inf/NaN
  if (((bits >> 23) & 0xFF) == 0xFF) {
    if (sign)
      *buf++ = '-';
    if (mantissa != 0) {
      *buf++ = 'N';
      *buf++ = 'a';
      *buf++ = 'N';
    } else {
      *buf++ = 'I';
      *buf++ = 'n';
      *buf++ = 'f';
    }
    *buf = '\0';
    return buf - start;
  }

  // Add implicit leading 1 for normalized floats
  if (((bits >> 23) & 0xFF) != 0) {
    mantissa |= (1 << 23);
  }

  // Multiply mantissa by SCALE and add rounding offset
  uint64_t scaled = (uint64_t)mantissa * SCALE + (SCALE / 2);

  int shift = exponent - 23;

  if (shift > 0) {
    if (shift < 41)
      scaled <<= shift;
    else
      scaled = 0;
  } else if (shift < 0) {
    if (-shift < 64)
      scaled >>= -shift;
    else
      scaled = 0;
  }

  if (sign)
    *buf++ = '-';

  uint32_t int_part = (uint32_t)(scaled / SCALE);
  uint32_t frac_part = (uint32_t)(scaled % SCALE);

  buf += utoa(int_part, buf, 10);

  *buf++ = '.';

  uint32_t divisor = SCALE / 10;
  while (divisor > 0) {
    *buf++ = '0' + (frac_part / divisor) % 10;
    divisor /= 10;
  }

  while (*(buf - 1) == '0' && *(buf - 2) != '.') {
    buf--;
  }

  *buf = '\0';
  return buf - start;
}
