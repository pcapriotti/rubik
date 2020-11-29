#ifndef PUZZLE_H
#define PUZZLE_H

struct symmetries_t {
  unsigned int *by_vertex;
  unsigned int *by_edge;

  uint32_t *face_action;
  uint8_t *vertex_action;
  uint8_t *edge_action;

  unsigned int *edges_by_face;

  /* multiplication table */
  uint8_t *mul;
  /* inverse multiplication table */
  uint8_t *inv_mul;
};

typedef struct symmetries_t symmetries_t;

#endif /* PUZZLE_H */
