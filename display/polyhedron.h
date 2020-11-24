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
struct abs_poly_t
{
  unsigned int num_vertices;
  unsigned int num_faces;
  face_t *faces;
  unsigned int *vertices;
  unsigned int len; /* size of the vertices array */
};
typedef struct abs_poly_t abs_poly_t;

/* realisation of a polyhedron */
struct poly_t
{
  abs_poly_t abs;
  vec3 *vertices;
};
typedef struct poly_t poly_t;

void abs_cube(abs_poly_t *cube);
void abs_dodec(abs_poly_t *dodec);
void abs_prism(abs_poly_t *prism, unsigned int num);

void abs_poly_debug(abs_poly_t *poly);

/* cube of side length 1 centred at the origin */
void std_cube(poly_t *poly);

/* dodecahedron of radius 1 centred at the origin */
void std_dodec(poly_t *poly);

/* prism with num sides */
void std_prism(poly_t *poly, unsigned int num);

void poly_debug(poly_t *poly);

#endif /* POLYHEDRON_H */
