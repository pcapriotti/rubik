#include "abs_poly.h"

#include "utils.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int abs_poly_num_edges(abs_poly_t *poly)
{
  unsigned total = 0;
  for (unsigned int f = 0; f < poly->num_faces; f++) {
    total += poly->faces[f].num_vertices;
  }
  return total / 2;
}

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
      cube->vertices[index++] = rotl3(~code & 0x7, (i + 1) % 3);
    }
  }
}

void abs_dodec(abs_poly_t *poly)
{
  poly->num_vertices = 20;
  poly->num_faces = 12;
  poly->faces = malloc(poly->num_faces * sizeof(face_t));
  poly->len = poly->num_faces * 5;
  poly->vertices = malloc(poly->len * sizeof(unsigned int));

  for (unsigned int i = 0; i < poly->num_faces; i++) {
    poly->faces[i].num_vertices = 5;
    poly->faces[i].vertices = poly->vertices + 5 * i;
  }

  /* vertices 0-4 belong to face 0, vertex 5+i is radially connected
  to vertex i for i=0..4, and vertex 19-i is opposite to vertex i */
  for (unsigned int i = 0; i < 5; i++) {
    poly->faces[0].vertices[i] = i;
    poly->faces[1].vertices[i] = 15 + i;
  }

  for (unsigned int j = 0; j < 5; j++) {
    poly->faces[j * 2 + 2].vertices[0] = (j + 1) % 5;
    poly->faces[j * 2 + 2].vertices[1] = j;
    poly->faces[j * 2 + 2].vertices[2] = j + 5;
    poly->faces[j * 2 + 2].vertices[3] = 14 - (j + 3) % 5;
    poly->faces[j * 2 + 2].vertices[4] = (j + 1) % 5 + 5;

    poly->faces[j * 2 + 3].vertices[4] = 19 - (j + 1) % 5;
    poly->faces[j * 2 + 3].vertices[3] = 19 - j;
    poly->faces[j * 2 + 3].vertices[2] = 14 - j;
    poly->faces[j * 2 + 3].vertices[1] = (j + 3) % 5 + 5;
    poly->faces[j * 2 + 3].vertices[0] = 14 - (j + 1) % 5;
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

unsigned int **abs_poly_edges_by_face(abs_poly_t *poly, int *edges, int *adj)
{
  unsigned int num_edges = abs_poly_num_edges(poly);
  unsigned int **table = malloc(poly->num_faces * sizeof(unsigned int *));

  unsigned int index = 0;

  for (unsigned int f = 0; f < poly->num_faces; f++) {
    table[f] = malloc(poly->faces[f].num_vertices * sizeof(unsigned int));
  }

  for (unsigned int f = 0; f < poly->num_faces; f++) {
    unsigned int n = poly->faces[f].num_vertices;
    for (unsigned int i = 0; i < n; i++) {
      unsigned int f1 = abs_poly_get_adj_face(poly, f, i, edges);
      if (f1 < f) continue;
      unsigned int v1 = poly->faces[f].vertices[(i + 1) % n];
      int i1 = adj[f1 * poly->num_vertices + v1];
      assert(i1 >= 0);
      table[f1][i1] = index;
      table[f][i] = index;
      index++;
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

void poly_data_init(poly_data_t *data, abs_poly_t *poly)
{
  data->num_faces = poly->num_faces;
  data->edges = abs_poly_edges(poly);
  data->first_vertex = abs_poly_first_vertex(poly, data->edges);
  data->adj = abs_poly_adj(poly);
  data->edges_by_face = abs_poly_edges_by_face(poly, data->edges, data->adj);
}

void poly_data_cleanup(poly_data_t *data)
{
  free(data->edges);
  free(data->first_vertex);
  free(data->adj);
  for (unsigned int f = 0; f < data->num_faces; f++) {
    free(data->edges_by_face[f]);
  }
  free(data->edges_by_face);
}

void abs_poly_cleanup(abs_poly_t *poly)
{
  free(poly->faces);
  free(poly->vertices);
}
