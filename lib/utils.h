#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

static inline uint8_t rotr3(uint8_t x, unsigned int n)
{
  return ((x >> n) | (x << (3 - n))) & 0x7;
}

static inline uint8_t rotl3(uint8_t x, unsigned int n)
{
  return ((x << n) | (x >> (3 - n))) & 0x7;
}

void perm_flip_parity(uint8_t *x);
void parity_shuffle(uint8_t *x, size_t len, uint8_t parity);

#endif /* UTILS_H */
