#include "group.h"

#include <stdio.h>
#include <stdlib.h>

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

struct group_memo_data_t
{
  unsigned int num;
  uint8_t *mul;
  uint8_t *inv_mul;
};

unsigned int group_memo_mul(void *data_, unsigned int x, unsigned int y)
{
  struct group_memo_data_t *data = data_;
  return data->mul[x + y * data->num];
}

unsigned int group_memo_inv_mul(void *data_, unsigned int x, unsigned int y)
{
  struct group_memo_data_t *data = data_;
  return data->inv_mul[x + y * data->num];
}

void group_memo_cleanup(void *data_)
{
  struct group_memo_data_t *data = data_;
  free(data->mul);
  free(data->inv_mul);
  free(data);
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

  struct group_memo_data_t *data = malloc(sizeof(struct group_memo_data_t));
  data->mul = table;
  data->inv_mul = inv_table;
  data->num = group->num;

  memo->num = group->num;
  memo->data = data;
  memo->mul = group_memo_mul;
  memo->inv_mul = group_memo_inv_mul;
  memo->cleanup = group_memo_cleanup;
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
