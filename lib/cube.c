#include "cube.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "group.h"
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
      y = (z > (n - 2) / 2) ? z : 1;
    }
  }
  if (n % 2 == 1) shape->orbits[shape->num_orbits - 1].size = 6;
}

void cube_shape_cleanup(cube_shape_t *shape)
{
  free(shape->orbits);
}

void cube_init(puzzle_t *puzzle, cube_t *cube, cube_shape_t *shape)
{
  cube->shape = shape;
  cube->pieces = malloc(cube->shape->num_pieces * sizeof(uint8_t));

  unsigned int index = 0;
  for (unsigned int i = 0; i < cube->shape->num_orbits; i++) {
    orbit_t *orbit = &cube->shape->orbits[i];
    for (unsigned int j = 0; j < orbit->size; j++) {
      cube->pieces[index++] = puzzle->by_stab[orbit->dim][j];
    }
  }
}

void cube_cleanup(cube_t *cube)
{
  free(cube->pieces);
}

uint8_t *cube_orbit(cube_t *cube, unsigned int k)
{
  return &cube->pieces[cube->shape->orbits[k].offset];
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
    return i == 0 && (((n >> (f >> 1)) & 1) != (f & 1));
  }
  /* TODO: edges and centres */
  return 0;
}

cube_t *cube_generators(cube_t *cube, puzzle_t *puzzle, unsigned int *num_gen)
{
  *num_gen = 6 * (cube->shape->n / 2);
  cube_t *gen = malloc(*num_gen * sizeof(cube_t));
  unsigned int index = 0;
  for (unsigned int f = 0; f < 6; f++) {
    /* cw rotation around axis of face 0 */
    unsigned int s = puzzle->by_stab[2][puzzle->orbit_size[2] * 3];
    s = group_conj(puzzle->group, s, puzzle->by_stab[2][f]);

    for (unsigned int i = 0; i < cube->shape->n / 2; i++) {
      gen[index].pieces = calloc(cube->shape->num_pieces, sizeof(uint8_t));
      gen[index].shape = cube->shape;
      for (unsigned int k = 0; k < cube->shape->num_orbits; k++) {
        for (unsigned int j = 0; j < cube->shape->orbits[k].size; j++) {
          if (piece_in_layer(cube->shape, k, j, f, i)) {
            gen[index].pieces[cube->shape->orbits[k].offset + j] = s;
          }
        }
      }
      index++;
    }
  }
  return gen;
}

static unsigned int act(puzzle_t *puzzle, orbit_t *orbit, unsigned int x, unsigned int g)
{
  unsigned int g0 = puzzle->by_stab[orbit->dim][x];
  unsigned int g1 = group_mul(puzzle->group, g0, g);
  unsigned int j = puzzle->inv_by_stab[orbit->dim][g1];
  return orbit->offset + j % orbit->size;
}

void cube_act(puzzle_t *puzzle, cube_t *conf1, cube_t *conf, cube_t *move)
{
  conf1->shape = conf->shape;
  for (unsigned int k = 0; k < conf->shape->num_orbits; k++) {
    for (unsigned int i = 0; i < conf->shape->orbits[k].size; i++) {
      unsigned int i1 = act(puzzle, &conf->shape->orbits[k], 0, conf->pieces[i]);
      conf1->pieces[i] = group_mul(puzzle->group,
                                   conf->pieces[i],
                                   move->pieces[i1]);
    }
  }
}

void cube_act_(puzzle_t *syms, cube_t *cube, cube_t *move)
{
  cube_act(syms, cube, cube, move);
}
