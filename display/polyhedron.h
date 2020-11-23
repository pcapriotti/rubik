#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <memory.h>
#include "linmath.h"

typedef struct
{
  unsigned int num_vertices;
  unsigned int *vertices;
} face_t;

/* abstract polyhedron */
typedef struct
{
  unsigned int num_vertices;
  unsigned int num_faces;
  face_t *faces;
  unsigned int *vertices;
} abs_poly_t;

/* realisation of a polyhedron */
typedef struct
{
  abs_poly_t abs;
  vec3 *vertices;
} poly_t;

void abs_cube(abs_poly_t *cube);
void abs_dodec(abs_poly_t *dodec);
void abs_poly_debug(abs_poly_t *poly);

/* cube of side length 1 centred at the origin */
void std_cube(poly_t *poly);

void poly_debug(poly_t *poly);

#endif /* POLYHEDRON_H */
