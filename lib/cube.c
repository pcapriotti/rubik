#include "cube.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "puzzle.h"

void cube_shape_init(cube_shape_t *shape, unsigned int n)
{
  assert(n >= 1);
  shape->n = n;
  shape->num_corners = n > 1 ? 8 : 0;
  shape->num_edges = 12 * (n - 2);
  shape->num_centres = n > 1 ? 6 * (n - 2) * (n - 2) : 1;
  shape->num_pieces = shape->num_corners + shape->num_edges + shape->num_centres;

  shape->num_corner_orbits = shape->num_corners != 0;
  shape->num_edge_orbits = (n - 1) / 2;
  shape->num_centre_orbits = (n - 2) * (n - 2) / 4;
  if (n % 2 == 1) shape->num_centre_orbits++;
  shape->num_orbits = shape->num_corner_orbits +
    shape->num_edge_orbits + shape->num_centre_orbits;

  unsigned int offset = 0;

  shape->orbits = malloc(shape->num_orbits * sizeof(orbit_t));
  /* only 1 corner orbit of size 8 */
  shape->orbits[0].size = shape->num_corners;
  shape->orbits[0].dim = 0;
  shape->orbits[0].offset = offset;
  shape->orbits[0].x = 0;
  shape->orbits[0].y = 0;
  shape->orbits[0].z = 0;
  offset += shape->orbits[0].size;

  /* edge orbits have size 24, except the middle one when n is odd */
  for (unsigned int i = 1; i <= shape->num_edge_orbits; i++) {
    shape->orbits[i].size = (i * 2 == n - 1) ? 12 : 24;
    shape->orbits[i].dim = 1;
    shape->orbits[i].offset = offset;
    shape->orbits[i].x = i;
    shape->orbits[i].y = 0;
    shape->orbits[i].z = 0;
    offset += shape->orbits[i].size;
  }

  /* centre orbits have size 24, except the central one when n is odd */
  unsigned int y = 1; unsigned int z = 1;
  for (unsigned int i = shape->num_edge_orbits + 1;
       i < shape->num_orbits; i++) {
    shape->orbits[i].size = 24;
    shape->orbits[i].dim = 2;
    shape->orbits[i].offset =  offset;
    offset += shape->orbits[i].size;
    shape->orbits[i].x = n - 1;
    shape->orbits[i].y = y++;
    shape->orbits[i].z = z;

    if (y > shape->num_edge_orbits) {
      z++;
      y = 1;
    }
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
    orbit_t *orbit = &cube->shape.orbits[i];
    for (unsigned int j = 0; j < orbit->size; j++) {
      cube->pieces[index++] = symmetries_by_cell(syms, orbit->dim,
                                                 j * 24 / orbit->size);
    }
  }
}

uint8_t *cube_orbit(cube_t *cube, unsigned int k)
{
  return &cube->pieces[cube->shape.orbits[k].offset];
}

/* k: orbit index
   n: index of piece in orbit
   f: face index
   i: layer index */
int piece_in_layer(cube_shape_t *shape,
                   unsigned int k, unsigned int n,
                   unsigned int f, unsigned int i)
{
  if (k < shape->num_corner_orbits) {
    return i == 0 && ((n >> (f >> 1)) == (f & 1));
  }
  return 0;
}

cube_t *cube_generators(cube_t *cube, symmetries_t *syms)
{
  cube_t *gen = calloc(6 * (cube->shape.n / 2), sizeof(cube_t));
  for (unsigned int f = 0; f < 6; f++) {
    unsigned int s = syms->by_face[f * 4];
    for (unsigned int i = 0; i < cube->shape.n / 2; i++) {


      for (unsigned int k = 0; k < cube->shape.num_orbits; k++) {
        for (unsigned int j = 0; j < cube->shape.orbits[k].size; j++) {
          if (piece_in_layer(&cube->shape, k, j, f, i))
            cube->pieces[k] = s;
        }
      }
    }
  }
  return gen;
}

void cube_act(symmetries_t *syms, cube_t *cube1, cube_t *cube, cube_t *move)
{
  for (unsigned int k = 0; k < cube->shape.num_orbits; k++) {
    for (unsigned int i = 0; i < cube->shape.orbits[k].size; i++) {

    }
  }
}

void cube_act_(symmetries_t *syms, cube_t *cube, cube_t *move)
{
  cube_act(syms, cube, cube, move);
}
