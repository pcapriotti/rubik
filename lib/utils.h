#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

static inline uint8_t rotr3(uint8_t x, unsigned int n)
{
  return ((x >> n) | (x << (3 - n))) & 0x7;
}

static inline uint8_t rotl3(uint8_t x, unsigned int n)
{
  return ((x << n) | (x >> (3 - n))) & 0x7;
}

#endif /* UTILS_H */
