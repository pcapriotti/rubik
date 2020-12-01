#include "group.h"

#include <stdio.h>

void group_inv_table(uint8_t *inv_mul, uint8_t *mul, unsigned int n)
{
  for (unsigned int a = 0; a < n; a++) {
    for (unsigned int b = 0; b < n; b++) {
      unsigned int c = mul[n * b + a];
      inv_mul[n * c + a] = b;
    }
  }
}
