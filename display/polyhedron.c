#include "polyhedron.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void abs_poly_debug(abs_poly_t *poly)
{
  printf("num_vertices: %u\n", poly->num_vertices);
  for (unsigned int i = 0; i < poly->num_faces; i++) {
    printf("face %u (%u): ", i, poly->faces[i].num_vertices);
    for (unsigned int j = 0; j < poly->faces[i].num_vertices; j++) {
      printf("%u ", poly->faces[i].vertices[j]);
    }
    printf("\n");
  }
}

uint8_t rotr3(uint8_t x, unsigned int n)
{
  return ((x >> n) | (x << (3 - n))) & 0x7;
}

void abs_cube(abs_poly_t *cube)
{
  cube->num_vertices = 8;
  cube->num_faces = 6;
  cube->faces = malloc(cube->num_faces * sizeof(face_t));
  cube->vertices = malloc(cube->num_faces * 4 * sizeof(unsigned int));

  for (unsigned int i = 0; i < cube->num_faces; i++) {
    cube->faces[i].num_vertices = 4;
    cube->faces[i].vertices = cube->vertices + 4 * i;
  }

  /* vertices are enumerated in base 2 */
  unsigned int index = 0;
  for (unsigned int i = 0; i < 3; i++) {
    /* vertices are laid out along opposite face pairs using a Grey code */
    for (unsigned int n = 0; n < 8; n++) {
      cube->vertices[index++] = rotr3(n ^ (n >> 1), i);
    }
  }
}

void abs_dodec(abs_poly_t *dodec)
{
  dodec->num_vertices = 20;
  dodec->num_faces = 12;
  dodec->faces = malloc(dodec->num_faces * sizeof(face_t));
  dodec->vertices = malloc(dodec->num_faces * 5 * sizeof(unsigned int));

  for (unsigned int i = 0; i < dodec->num_faces; i++) {
    dodec->faces[i].num_vertices = 5;
    dodec->faces[i].vertices = dodec->vertices + 5 * i;
  }

  /* vertices 0-4 belong to face 0, vertex 5+i is radially connected
  to vertex i for i=0..4, and vertex 19-i is opposite to vertex i */
  for (unsigned int i = 0; i < 5; i++) {
    dodec->faces[0].vertices[i] = i;
    dodec->faces[11].vertices[i] = 15 + i;
  }

  for (unsigned int j = 0; j < 5; j++) {
    dodec->faces[j + 1].vertices[0] = (j + 1) % 5;
    dodec->faces[j + 1].vertices[1] = j;
    dodec->faces[j + 1].vertices[2] = j + 5;
    dodec->faces[j + 1].vertices[3] = 14 - (j + 3) % 5;
    dodec->faces[j + 1].vertices[4] = (j + 1) % 5 + 5;

    dodec->faces[10 - j].vertices[4] = 19 - (j + 1) % 5;
    dodec->faces[10 - j].vertices[3] = 19 - j;
    dodec->faces[10 - j].vertices[2] = 14 - j;
    dodec->faces[10 - j].vertices[1] = (j + 3) % 5 + 5;
    dodec->faces[10 - j].vertices[0] = 14 - (j + 1) % 5;
  }
}

void poly_debug(poly_t *poly)
{
  abs_poly_debug(&poly->abs);
  for (unsigned int i = 0; i < poly->abs.num_vertices; i++) {
    printf("(%.2f, %.2f, %.2f) ", poly->vertices[i][0], poly->vertices[i][1], poly->vertices[i][2]);
  }
  printf("\n");
}

void std_cube(poly_t *poly)
{
  abs_cube(&poly->abs);
  poly->vertices = malloc(poly->abs.num_vertices * sizeof(vec3));

  for (unsigned int n = 0; n < poly->abs.num_vertices; n++) {
    for (unsigned int i = 0; i < 3; i++) {
      poly->vertices[n][i] = (float)((n >> i) & 1) - 0.5f;
    }
  }
}
