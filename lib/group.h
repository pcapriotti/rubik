#ifndef GROUP_H
#define GROUP_H

#include <stdint.h>

/* A group structure on the natural numbers below num.
   mul is the operation of the group, inv the inverse. The unit
   element is 0. */
struct group_t
{
  unsigned int num;
  unsigned int (*mul)(void *data, unsigned int x, unsigned int y);
  unsigned int (*inv)(void *data, unsigned int x);
  void *data;
};
typedef struct group_t group_t;

/* An action of a group on a finite set of natural numbers. */
struct action_t
{
  unsigned int (*act)(void *data, unsigned int a, unsigned int g);
  void *data;
};
typedef struct action_t action_t;

void group_inv_table(uint8_t *inv_mul, uint8_t *mul, unsigned int n);

#endif /* GROUP_H */
