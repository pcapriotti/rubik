#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <memory.h>
#include "linmath.h"

/* realisation of a polyhedron */
struct poly_t;
typedef struct poly_t poly_t;


void megaminx_corner(poly_t *mm, poly_t *dodec, float edge);
void megaminx_edge(poly_t *mm, poly_t *dodec, float edge);
void megaminx_centre(poly_t *mm, poly_t *dodec, float edge);

static const unsigned int megaminx_num_syms = 60;
typedef struct {
  quat *syms;
  unsigned int *by_vertex;
  unsigned int *by_edge;

  /* unsigned int *face_action; */
  /* unsigned int *vertex_action; */
  /* unsigned int *edge_action; */
} symmetries_t;

void gen_megaminx_syms(symmetries_t *syms, poly_t *dodec);

#endif /* MEGAMINX_H */
