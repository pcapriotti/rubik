#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <stdint.h>

static const unsigned int megaminx_num_syms = 60;

#define MEGAMINX_NUM_CORNERS 20
#define MEGAMINX_NUM_EDGES 30
#define MEGAMINX_NUM_CENTRES 12

typedef struct {
  unsigned int *by_vertex;
  unsigned int *by_edge;

  uint32_t *face_action;
} symmetries_t;

typedef struct
{
  unsigned int corners[MEGAMINX_NUM_CORNERS];
  unsigned int edges[MEGAMINX_NUM_EDGES];
  unsigned int centres[MEGAMINX_NUM_CENTRES];
} megaminx_t;

void megaminx_init(megaminx_t *mm);

#endif /* MEGAMINX_H */
