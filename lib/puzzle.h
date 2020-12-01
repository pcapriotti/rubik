#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdint.h>

struct symmetries_t {
  unsigned int *by_face;
  unsigned int *by_vertex;
  unsigned int *by_edge;

  uint8_t *face_action;
  uint8_t *vertex_action;
  uint8_t *edge_action;

  unsigned int *edges_by_face;

  /* multiplication table */
  uint8_t *mul;
  /* inverse multiplication table */
  uint8_t *inv_mul;
};

typedef struct symmetries_t symmetries_t;

unsigned int symmetries_by_cell(symmetries_t *syms, unsigned int dim, unsigned int i);

#endif /* PUZZLE_H */
