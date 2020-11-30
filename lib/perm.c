#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "perm.h"

void perm_id(uint8_t *x, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    x[i] = i;
  }
}

void perm_inverted(uint8_t *y, uint8_t *x, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    y[x[i]] = i;
  }
}

void perm_composed(uint8_t *r, uint8_t *x, uint8_t *y, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    r[i] = x[y[i]];
  }
}

void perm_composed_inv(uint8_t *r, uint8_t *x, uint8_t *y, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    r[y[i]] = x[i];
  }
}

void perm_mul(uint8_t *x, uint8_t *p, size_t len)
{
  uint8_t *y = malloc(len);
  memcpy(y, x, len);
  perm_composed(x, y, p, len);
  free(y);
}

void perm_mul_inv(uint8_t *x, uint8_t *p, size_t len)
{
  uint8_t *y = malloc(len);
  memcpy(y, x, len);
  perm_composed_inv(x, y, p, len);
  free(y);
}

void perm_lmul(uint8_t *x, uint8_t *p, size_t len)
{
  uint8_t *y = malloc(len);
  memcpy(y, x, len);
  perm_composed(x, p, y, len);
  free(y);
}

void perm_lmul_inv(uint8_t *x, uint8_t *p, size_t len)
{
  uint8_t *y = malloc(len);
  perm_inverted(y, p, len);
  perm_lmul(x, y, len);
  free(y);
}

void perm_inv(uint8_t *x, size_t len)
{
  uint8_t *y = malloc(len);
  memcpy(y, x, len);
  perm_inverted(x, y, len);
}

/* generate lehmer code of x */
void perm_lehmer(uint8_t *lehmer, uint8_t *x, size_t len)
{
  uint32_t visited = 0;
  for (size_t i = 0; i < len; i++) {
    unsigned int bit = 1 << x[i];
    lehmer[i] = x[i] - __builtin_popcount(visited & (bit - 1));
    visited |= bit;
  }
}

/* reconstruct permutation from lehmer code */
void perm_from_lehmer(uint8_t *x, uint8_t *lehmer, size_t len)
{
  uint32_t visited = 0;
  for (size_t i = 0; i < len; i++) {
    int value = 0;
    int l = lehmer[i];

    while (visited & (1 << value)) { value++; }
    while (l--) {
       do { value++; } while (visited & (1 << value));
    }
    visited |= (1 << value);
    x[i] = value;
  }
}

int perm_index(uint8_t *x, size_t len, size_t n)
{
  uint8_t *lehmer = malloc(len);
  perm_lehmer(lehmer, x, len);

  int index = lehmer_index(lehmer, len, n);

  free(lehmer);

  return index;
}

int lehmer_index(uint8_t *lehmer, size_t len, size_t n)
{
  int index = 0;
  int b = 1;
  for (size_t i = n - len + 1; i <= n; i++) {
    uint8_t l = i == 1 ? 0 : lehmer[n - i];
    index += l * b;
    b *= i;
  }

  return index;
}

uint8_t perm_sign(uint8_t *x, size_t len)
{
  uint8_t *lehmer = malloc(len);
  perm_lehmer(lehmer, x, len);

  uint8_t sign = lehmer_sign(lehmer, len);

  free(lehmer);

  return sign;
}

uint8_t lehmer_sign(uint8_t *lehmer, size_t len)
{
  uint8_t sign = 0;
  for (unsigned int i = 0; i < len; i++) {
    sign += lehmer[i];
  }
  return sign % 2;
}

void perm_from_index(uint8_t *x, size_t len, int index, size_t n)
{
  uint8_t *lehmer = malloc(len);
  lehmer_from_index(lehmer, len, index, n);
  perm_from_lehmer(x, lehmer, len);
  free(lehmer);
}

void lehmer_from_index(uint8_t *lehmer, size_t len, int index, size_t n)
{
  memset(lehmer, 0, len);

  int b = n - len + 1;
  for (size_t i = n - len + 2; i <= n - 1; i++) {
    b *= i;
  }

  for (size_t i = 0; i < len && i < n - 1; i++) {
    lehmer[i] = index / b;
    index = index % b;
    b /= (n - i - 1);
  }
}

void perm_conj(uint8_t *x, uint8_t *y, size_t len)
{
  perm_mul(x, y, len);
  perm_lmul_inv(x, y, len);
}

uint16_t u16_conj(uint16_t word, uint8_t *p)
{
  uint16_t ret = 0;
  for (int i = 0; i < 12; i++) {
    ret |= (word & (1 << p[i])) >> p[i] << i;
  }
  return ret;
}

uint16_t u16_conj_inv(uint16_t word, uint8_t *p)
{
  uint16_t ret = 0;
  for (int i = 0; i < 12; i++) {
    ret |= (word & (1 << i)) >> i << p[i];
  }
  return ret;
}

void shuffle(uint8_t *x, size_t len)
{
  for (size_t i = 0; i < len - 1; i++) {
    int j = rand() % (len - i);
    uint8_t tmp = x[i];
    x[i] = x[i + j];
    x[i + j] = tmp;
  }
}

void debug_perm(uint8_t *x, size_t len)
{
  for (unsigned int i = 0; i < len; i++) {
    printf("%u ", x[i]);
  }
}
