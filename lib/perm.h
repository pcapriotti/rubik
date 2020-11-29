#ifndef PERM_H
#define PERM_H

#include <stddef.h>
#include <stdint.h>

/* set x to the identity permutation */
void perm_id(uint8_t *x, size_t len);

/* set r to x y */
void perm_composed(uint8_t *r, uint8_t *x, uint8_t *y, size_t len);

/* set r to x y' */
void perm_composed_inv(uint8_t *r, uint8_t *x, uint8_t *y, size_t len);

/* set y to x' */
void perm_inverted(uint8_t *y, uint8_t *x, size_t len);

/* replace x with x p */
void perm_mul(uint8_t *x, uint8_t *p, size_t len);

/* replace x with x p' */
void perm_mul_inv(uint8_t *x, uint8_t *p, size_t len);

/* replace x with p x */
void perm_lmul(uint8_t *x, uint8_t *p, size_t len);

/* replace x with x p' */
void perm_lmul_inv(uint8_t *x, uint8_t *p, size_t len);

/* replace x with x' */
void perm_inv(uint8_t *x, size_t len);

int perm_index(uint8_t *x, size_t len, size_t n);
void perm_from_index(uint8_t *x, size_t len, int index, size_t n);

/* apply permutation to a 16 bit word */
uint16_t u16_conj(uint16_t word, uint8_t *p);
uint16_t u16_conj_inv(uint16_t word, uint8_t *p);

/* conjugation */
void perm_conj(uint8_t *x, uint8_t *y, size_t len);

/* random permutation */
void shuffle(uint8_t *x, size_t len);

/* sign as integer mod 2 */
uint8_t perm_sign(uint8_t *x, size_t len);

void debug_perm(uint8_t *x, size_t len);

#endif /* PERM_H */
