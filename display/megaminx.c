#include "megaminx.h"
#include "polyhedron.h"

#include <memory.h>
#include <stdlib.h>
#include "linmath.h"

/* extract a megaminx corner from vertex 0 of a dodecahedron */
/* edge is the ratio between the corner edge length and the full edge length */
void megaminx_corner(poly_t *mm, poly_t *dodec, float edge)
{
  abs_prism(&mm->abs, 4);

  vec3 p0, v1, v2, v3;
  memcpy(p0, dodec->vertices[0], sizeof(p0));

  vec3_sub(v1, dodec->vertices[1], p0); vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[4], p0); vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[5], p0); vec3_scale(v3, v3, edge);

  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));
  for (unsigned int i = 0; i < 8; i++) {
    memcpy(mm->vertices[i], p0, sizeof(vec3));
    if (i % 4 == 1 || i % 4 == 2) vec3_add(mm->vertices[i], mm->vertices[i], v1);
    if (i % 4 == 2 || i % 4 == 3) vec3_add(mm->vertices[i], mm->vertices[i], v2);
    if (i < 4) vec3_add(mm->vertices[i], mm->vertices[i], v3);
  }
}
