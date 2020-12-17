#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <memory.h>
#include "linmath.h"

#include "lib/abs_poly.h"

/* realisation of a polyhedron */
struct poly_t
{
  abs_poly_t abs;
  vec3 *vertices;
};
typedef struct poly_t poly_t;

/* cube of side length 1 centred at the origin */
void std_cube(poly_t *poly);

/* dodecahedron of radius 1 centred at the origin */
void std_dodec(poly_t *poly);

/* tetrahedron of radius 1 centred at the origin */
void std_tetra(poly_t *poly);

/* prism with num sides */
void std_prism(poly_t *poly, unsigned int num);

void poly_debug(poly_t *poly);
void poly_cleanup(poly_t *poly);

#endif /* POLYHEDRON_H */
