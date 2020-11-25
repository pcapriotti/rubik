#include "polyhedron.h"

#include <assert.h>
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
  cube->len = cube->num_faces * 4;
  cube->vertices = malloc(cube->len * sizeof(unsigned int));

  for (unsigned int i = 0; i < cube->num_faces; i++) {
    cube->faces[i].num_vertices = 4;
    cube->faces[i].vertices = cube->vertices + 4 * i;
  }

  /* vertices are enumerated in base 2 */
  unsigned int index = 0;
  for (unsigned int i = 0; i < 3; i++) {
    /* vertices are laid out along opposite face pairs using a Grey code */
    for (unsigned int n = 0; n < 8; n++) {
      uint8_t code = n ^ (n >> 1);
      cube->vertices[index++] = rotr3(~code & 0x7, i);
    }
  }
}

void abs_dodec(abs_poly_t *dodec)
{
  dodec->num_vertices = 20;
  dodec->num_faces = 12;
  dodec->faces = malloc(dodec->num_faces * sizeof(face_t));
  dodec->len = dodec->num_faces * 5;
  dodec->vertices = malloc(dodec->len * sizeof(unsigned int));

  for (unsigned int i = 0; i < dodec->num_faces; i++) {
    dodec->faces[i].num_vertices = 5;
    dodec->faces[i].vertices = dodec->vertices + 5 * i;
  }

  /* vertices 0-4 belong to face 0, vertex 5+i is radially connected
  to vertex i for i=0..4, and vertex 19-i is opposite to vertex i */
  for (unsigned int i = 0; i < 5; i++) {
    dodec->faces[0].vertices[i] = i;
    dodec->faces[1].vertices[i] = 15 + i;
  }

  for (unsigned int j = 0; j < 5; j++) {
    dodec->faces[j * 2 + 2].vertices[0] = (j + 1) % 5;
    dodec->faces[j * 2 + 2].vertices[1] = j;
    dodec->faces[j * 2 + 2].vertices[2] = j + 5;
    dodec->faces[j * 2 + 2].vertices[3] = 14 - (j + 3) % 5;
    dodec->faces[j * 2 + 2].vertices[4] = (j + 1) % 5 + 5;

    dodec->faces[j * 2 + 3].vertices[4] = 19 - (j + 1) % 5;
    dodec->faces[j * 2 + 3].vertices[3] = 19 - j;
    dodec->faces[j * 2 + 3].vertices[2] = 14 - j;
    dodec->faces[j * 2 + 3].vertices[1] = (j + 3) % 5 + 5;
    dodec->faces[j * 2 + 3].vertices[0] = 14 - (j + 1) % 5;
  }
}

void abs_prism(abs_poly_t *prism, unsigned int num)
{
  prism->num_vertices = num * 2;
  prism->num_faces = 2 + num;
  prism->len = 2 * num + num * 4;
  prism->vertices = malloc(prism->len * sizeof(unsigned int));
  prism->faces = malloc(prism->num_faces * sizeof(face_t));

  /* bases */
  unsigned int index = 0;
  prism->faces[0].num_vertices = num;
  prism->faces[0].vertices = prism->vertices;
  prism->faces[num + 1].num_vertices = num;
  prism->faces[num + 1].vertices = prism->vertices + num * 5;

  for (unsigned int i = 0; i < num; i++) {
    prism->faces[0].vertices[i] = num - i - 1;
    prism->faces[num + 1].vertices[i] = num + i;

    prism->faces[i + 1].num_vertices = 4;
    prism->faces[i + 1].vertices = prism->vertices + num + i * 4;
    prism->faces[i + 1].vertices[0] = i;
    prism->faces[i + 1].vertices[1] = (i + 1) % num;
    prism->faces[i + 1].vertices[2] = (i + 1) % num + num;
    prism->faces[i + 1].vertices[3] = i + num;
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

void std_prism(poly_t *poly, unsigned int num)
{
  abs_prism(&poly->abs, num);
  poly->vertices = malloc(poly->abs.num_vertices * sizeof(vec3));

  mat4x4 rho;
  mat4x4_identity(rho);
  mat4x4_rotate_Z(rho, rho, 2 * M_PI / num);

  vec4 v0 = { 1, 0, -1, 1 };

  for (unsigned int i = 0; i < num; i++) {
    vec4 v;
    memcpy(poly->vertices[i], v0, sizeof(vec3));
    memcpy(poly->vertices[i + num], v0, sizeof(vec3));
    poly->vertices[i + num][2] = 1;

    memcpy(v, v0, sizeof(v));
    mat4x4_mul_vec4(v0, rho, v);
  }
}

void std_dodec(poly_t *poly)
{
  abs_dodec(&poly->abs);
  poly->vertices = malloc(poly->abs.num_vertices * sizeof(vec3));

  float phi = (sqrt(5) + 1) / 2.0;
  float r = 2.0 / (sqrt(3) * phi * sqrt(3 - phi));
  float z = r * (phi + 1) / 2.0;

  mat4x4 rho;
  mat4x4_identity(rho);
  mat4x4_rotate_Z(rho, rho, 2 * M_PI / 5);

  vec4 v0 = { r, 0, z, 1};
  vec4 v5 = { r * phi, 0, z - r, 1 };

  for (unsigned int i = 0; i < 5; i++) {
    vec4 v;

    memcpy(poly->vertices[i], v0, sizeof(vec3));
    memcpy(v, v0, sizeof(v));
    mat4x4_mul_vec4(v0, rho, v);

    memcpy(poly->vertices[i + 5], v5, sizeof(vec3));
    memcpy(v, v5, sizeof(v));
    mat4x4_mul_vec4(v5, rho, v);

    vec3_scale(poly->vertices[19 - i], poly->vertices[i], -1);
    vec3_scale(poly->vertices[14 - i], poly->vertices[i + 5], -1);
  }
}

/* generate adjacency face/vertex adjacency matrix: for every face,
and every vertex, the index of that vertex in the face, or -1 if not
contained */
int *abs_poly_adj(abs_poly_t *poly)
{
  int *table = malloc(poly->num_vertices * poly->num_faces * sizeof(int));
  for (unsigned int i = 0; i < poly->num_vertices * poly->num_faces; i++) {
    table[i] = -1;
  }

  for (unsigned int j = 0; j < poly->num_faces; j++) {
    for (unsigned int i = 0; i < poly->faces[j].num_vertices; i++) {
      table[j * poly->num_vertices + poly->faces[j].vertices[i]] = i;
    }
  }

  return table;
}

/* for each pair of vertices connected by an edge, the face to the
left of the vector from the first to the second vertex */
int *abs_poly_edges(abs_poly_t *poly)
{
  int *table = malloc(poly->num_vertices * poly->num_vertices * sizeof(int));

  for (unsigned int i = 0; i < poly->num_vertices * poly->num_vertices; i++) {
    table[i] = -1;
  }

  for (unsigned int j = 0; j < poly->num_faces; j++) {
    unsigned int n = poly->faces[j].num_vertices;
    for (unsigned int i = 0; i < n; i++) {
      unsigned int v0 = poly->faces[j].vertices[i];
      unsigned int v1 = poly->faces[j].vertices[(i + 1) % n];

      table[v0 * poly->num_vertices + v1] = j;
    }
  }

  return table;
}

/* map every face to the vertex before its first adjacent face */
int *abs_poly_first_vertex(abs_poly_t *poly, int *edges)
{
  int *table = malloc(poly->num_faces * sizeof(unsigned int));

  for (unsigned int i = 0; i < poly->num_faces; i++) {
    table[i] = -1;
  }

  unsigned int num = 0;
  for (unsigned int j = 0; j < poly->num_faces; j++) {
    unsigned int n = poly->faces[j].num_vertices;
    for (unsigned int i = 0; i < n; i++) {
      int f = abs_poly_get_adj_face(poly, j, i, edges);
      assert(f != -1);
      if (table[f] != -1) continue;

      table[f] = poly->faces[j].vertices[(i + 1) % n];
      num++;
      if (num == poly->num_faces) return table;
    }
  }

  return table;
}

int abs_poly_get_adj_face(abs_poly_t *poly,
                          unsigned int f, int i,
                          int *edges)
{
  unsigned int n = poly->faces[f].num_vertices;
  i = (i % (int) n + n) % n;
  unsigned int v0 = poly->faces[f].vertices[i];
  unsigned int v1 = poly->faces[f].vertices[(i + 1) % n];
  return edges[v1 * poly->num_vertices + v0];
}
