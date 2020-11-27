#include "megaminx.h"

void megaminx_init(symmetries_t *syms, megaminx_t *mm)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    mm->corners[i] = syms->by_vertex[i * 3];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_EDGES; i++) {
    mm->edges[i] = syms->by_edge[i * 2];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_CENTRES; i++) {
    mm->centres[i] = i * 5;
  }
}

void megaminx_act_(megaminx_t *conf, megaminx_t *move)
{
}
