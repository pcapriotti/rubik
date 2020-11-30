#include "cube.h"

#include <assert.h>
#include <stdlib.h>

#include "puzzle.h"

void cube_shape_init(cube_shape_t *shape, unsigned int n)
{
  assert(n >= 1);
  shape->n = n;
  shape->num_corners = n > 1 ? 8 : 0;
  shape->num_edges = 12 * (n - 2);
  shape->num_centres = 6 * (n - 2) * (n - 2);
}

uint8_t *cube_corner(cube_t *cube, unsigned int c)
{
  return &cube->pieces[c];
}

uint8_t *cube_edge(cube_t *cube, unsigned int e)
{
  return &cube->pieces[e + cube->shape.num_corners];
}

uint8_t *cube_centre(cube_t *cube, unsigned int f)
{
  return &cube->pieces[f + cube->shape.num_corners + cube->shape.num_edges];
}

void cube_init(symmetries_t *syms, cube_t *cube, unsigned int n)
{
  unsigned int num;
  cube_shape_init(&cube->shape, n);
  cube->pieces = malloc((cube->shape.num_corners +
                         cube->shape.num_edges +
                         cube->shape.num_centres) * sizeof(uint8_t));

  for (unsigned int i = 0; i < cube->shape.num_corners; i++) {
    *cube_corner(cube, i) = syms->by_vertex[i * 3];
  }
  num = n - 2;
  for (unsigned int i = 0; i < cube->shape.num_edges; i++) {
    *cube_edge(cube, i) = syms->by_edge[i / num * 2];
  }
  num *= num;
  for (unsigned int i = 0; i < cube->shape.num_edges; i++) {
    *cube_centre(cube, i) = i / num * 4;
  }
}
