#include "group.h"

#include <stdio.h>
#include <stdlib.h>

#include "perm.h"

void group_cleanup(group_t *group)
{
  if (group->cleanup) group->cleanup(group->data);
}

unsigned int group_mul(group_t *group, unsigned int x, unsigned int y)
{
  return group->mul(group->data, x, y);
}

unsigned int group_inv_mul(group_t *group, unsigned int x, unsigned int y)
{
  return group->inv_mul(group->data, x, y);
}

unsigned int group_inv(group_t *group, unsigned int x, unsigned int y)
{
  return group->inv_mul(group->data, x, 0);
}

void action_cleanup(action_t *action)
{
  if (action->cleanup) action->cleanup(action->data);
}

struct group_memo_data_t
{
  unsigned int num;
  uint8_t *mul;
  uint8_t *inv_mul;
};

static unsigned int group_memo_mul(void *data_, unsigned int x, unsigned int y)
{
  struct group_memo_data_t *data = data_;
  return data->mul[x + y * data->num];
}

static unsigned int group_memo_inv_mul(void *data_, unsigned int x, unsigned int y)
{
  struct group_memo_data_t *data = data_;
  return data->inv_mul[x + y * data->num];
}

static void group_memo_cleanup(void *data_)
{
  struct group_memo_data_t *data = data_;
  free(data->mul);
  free(data->inv_mul);
  free(data);
}

void group_from_tables(group_t *memo,
                       unsigned int num,
                       uint8_t *mul,
                       uint8_t *inv_mul)
{
  struct group_memo_data_t *data = malloc(sizeof(struct group_memo_data_t));
  data->mul = mul;
  data->inv_mul = inv_mul;
  data->num = num;

  memo->num = num;
  memo->data = data;
  memo->mul = group_memo_mul;
  memo->inv_mul = group_memo_inv_mul;
  memo->cleanup = group_memo_cleanup;
}

void group_from_table(group_t *memo,
                      unsigned int num,
                      uint8_t *mul)
{
  uint8_t *inv_mul = malloc(num * num);
  for (unsigned int x = 0; x < num; x++) {
    for (unsigned int y = 0; y < num; y++) {
      unsigned int z = mul[x + y * num];
      inv_mul[x + z * num] = y;
    }
  }

  group_from_tables(memo, num, mul, inv_mul);
}

void group_memo(group_t *memo, group_t *group)
{
  uint8_t *table = malloc(group->num * group->num * sizeof(uint8_t));
  uint8_t *inv_table = malloc(group->num * group->num * sizeof(uint8_t));

  for (unsigned int x = 0; x < group->num; x++) {
    for (unsigned int y = 0; y < group->num; y++) {
      unsigned int z = group_mul(group, x, y);
      table[x + y * group->num] = z;
      inv_table[x + z * group->num] = y;
    }
  }

  group_from_tables(memo, group->num, table, inv_table);
}

static unsigned int group_perm_mul(void *data_, unsigned int x, unsigned int y)
{
  unsigned int *data = data_;
  unsigned int n = *data;

  uint8_t *perm1 = malloc(n);
  uint8_t *perm2 = malloc(n);

  perm_from_index(perm1, n, x, n);
  perm_from_index(perm2, n, y, n);

  perm_mul(perm1, perm2, n);

  unsigned int z = perm_index(perm1, n, n);

  free(perm1);
  free(perm2);

  return z;
}

void group_perm(group_t *group, unsigned int n)
{
  group->num = 1;
  for (unsigned int i = 2; i < n; i++) group->num *= i;

  unsigned int *data = malloc(sizeof(unsigned int));
  *data = n;
  group->data = data;
  group->cleanup = free;
  group->mul = group_perm_mul;
  group->inv_mul = 0;
}

static unsigned int group_u16_mul(void *data, unsigned int x, unsigned int y)
{
  return x ^ y;
}

void group_u16(group_t *group, unsigned int n)
{
  group->num = 1 << n;
  group->data = 0;
  group->cleanup = free;
  group->mul = group_u16_mul;
  group->inv_mul = group_u16_mul;
}

void group_inv_table(uint8_t *inv_mul, uint8_t *mul, unsigned int n)
{
  for (unsigned int a = 0; a < n; a++) {
    for (unsigned int b = 0; b < n; b++) {
      unsigned int c = mul[n * b + a];
      inv_mul[n * c + a] = b;
    }
  }
}

unsigned int action_perm_u16_act(void *data_, unsigned int a, unsigned int p)
{
  unsigned int *data = data_;
  unsigned int n = *data;
  uint8_t *perm = malloc(n);
  perm_from_index(perm, n, p, n);

  unsigned int a1 = u16_conj(a, perm, n);

  free(perm);

  return a1;
}

void action_perm_u16(action_t *action, group_t *perm)
{
  unsigned int *data = malloc(sizeof(unsigned int));

  action->act = action_perm_u16_act;
  action->cleanup = free;
  action->data = data;
}
