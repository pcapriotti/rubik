#include "utils.h"
#include "perm.h"

void perm_flip_parity(uint8_t *x)
{
  uint8_t tmp = x[0];
  x[0] = x[1];
  x[1] = tmp;
}

void parity_shuffle(uint8_t *x, size_t len, uint8_t parity)
{
  shuffle(x, len);
  uint8_t s = perm_sign(x, len);
  if (s ^ parity) {
    perm_flip_parity(x);
  }
}
