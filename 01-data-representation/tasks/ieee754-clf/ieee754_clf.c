#include "ieee754_clf.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

float_class_t classify(double x) {
  uint64_t bits = 0;
  memcpy(&bits, &x, sizeof(x));

  uint64_t exponent_max = (1ull << 11) - 1;
  uint64_t mantissa_max = (1ull << 52) - 1;

  uint64_t sign = bits >> 63;
  uint64_t exponent = (bits >> 52) & exponent_max;
  uint64_t mantissa = bits & mantissa_max;

  if (exponent == 0 && mantissa == 0) {
    return sign ? MinusZero : Zero;
  }

  if (exponent == 0) {
    return sign ? MinusDenormal : Denormal;
  }

  if (mantissa == 0 && exponent == exponent_max) {
    return sign ? MinusInf : Inf;
  }

  if (exponent == exponent_max && mantissa != 0) {
    return NaN;
  }

  return sign ? MinusRegular : Regular;
}

// int main() {
//   double x = 0;
//   scanf("%lf", &x);
//   float_class_t res = classify(x);
//   printf("%x\n", res);
// }
