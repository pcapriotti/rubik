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
  shape->num_pieces = shape->num_corners + shape->num_edges + shape->num_centres;

  unsigned int num_edge_orbits = (n - 1) / 2;
  unsigned int num_centre_orbits = num_edge_orbits * (num_edge_orbits + 1) / 2;
  shape->num_orbits = 1 + num_edge_orbits + num_centre_orbits;

  shape->orbits = malloc(shape->num_orbits * sizeof(orbit_t));
  /* only 1 corner orbit of size 8 */
  shape->orbits[0].size = 8;
  shape->orbits[1].dim = 0;

  /* edge orbits have size 24, except the middle one when n is odd */
  for (unsigned int i = 1; i <= num_edge_orbits; i++) {
    shape->orbits[i].size = (i * 2 == n - 1) ? 12 : 24;
    shape->orbits[i].dim = 1;
  }

  /* centre orbits have size 24, except the central one when n is odd */
  for (unsigned int i = num_edge_orbits + 1; i < shape->num_orbits; i++) {
    shape->orbits[i].size = 24;
    shape->orbits[i].dim = 1;
  }
  if (n % 2 == 1) shape->orbits[shape->num_orbits - 1].size = 6;
}

void cube_init(symmetries_t *syms, cube_t *cube, unsigned int n)
{
  unsigned int num;
  cube_shape_init(&cube->shape, n);
  cube->pieces = malloc(cube->shape.num_pieces * sizeof(uint8_t));

  unsigned int index = 0;
  for (unsigned int i = 0; i < cube->shape.num_orbits; i++) {
    for (unsigned int j = 0; j < cube->shape.orbits[i].size; j++) {
      index++;
    }
  }

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
