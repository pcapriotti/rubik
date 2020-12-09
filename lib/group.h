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
  unsigned int (*inv_mul)(void *data, unsigned int x, unsigned int y);
  void (*cleanup)(void *data);
  void *data;
};
typedef struct group_t group_t;

unsigned int group_inv(group_t *group, unsigned int x);
unsigned int group_inv_mul(group_t *group, unsigned int x, unsigned int y);
unsigned int group_mul(group_t *group, unsigned int x, unsigned int y);
unsigned int group_conj(group_t *group, unsigned int x, unsigned int y);
void group_cleanup(group_t *group);

/* An action of a group on a finite set of natural numbers. */
struct action_t
{
  unsigned int (*act)(void *data, unsigned int a, unsigned int g);
  void (*cleanup)(void *data);
  void *data;
};
typedef struct action_t action_t;

unsigned int action_act(action_t *action, unsigned int a, unsigned int g);
void action_cleanup(action_t *action);

void group_inv_table(uint8_t *inv_mul, uint8_t *mul, unsigned int n);
void group_from_table(group_t *memo, unsigned int num, uint8_t *mul);
void group_cyclic_subgroup(group_t *group,
                           uint8_t *elems, unsigned int num,
                           unsigned int gen);

#endif /* GROUP_H */
