#include "cube.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "group.h"
#include "puzzle.h"
#include "utils.h"

void cube_shape_init(cube_shape_t *shape, unsigned int n)
{
  assert(n >= 1);
  shape->n = n;

  unsigned int num_corners = n > 1 ? 8 : 0;
  unsigned int num_edges = 12 * (n - 2);
  unsigned int num_centres = n > 1 ? 6 * (n - 2) * (n - 2) : 1;
  shape->decomp.num_pieces = num_corners + num_edges + num_centres;

  unsigned int num_corner_orbits = num_corners != 0;
  unsigned int num_edge_orbits = (n - 1) / 2;
  unsigned int num_centre_orbits = (n - 2) * (n - 2) / 4;
  if (n % 2 == 1) num_centre_orbits++;
  unsigned int num_orbits = num_corner_orbits + num_edge_orbits + num_centre_orbits;
  shape->decomp.num_orbits = num_orbits;

  unsigned int offset = 0;

  shape->orbits = malloc(num_orbits * sizeof(orbit_t));
  shape->decomp.orbit_size = malloc(num_orbits * sizeof(unsigned int));
  shape->decomp.orbit_offset = malloc(num_orbits * sizeof(unsigned int));
  /* only 1 corner orbit of size 8 */
  shape->decomp.orbit_size[0] = num_corners;
  shape->decomp.orbit_offset[0] = offset;
  shape->orbits[0].dim = 0;
  shape->orbits[0].x = 0;
  shape->orbits[0].y = 0;
  shape->orbits[0].z = 0;
  offset += shape->decomp.orbit_size[0];

  /* edge orbits have size 24, except the middle one when n is odd */
  for (unsigned int i = 1; i <= num_edge_orbits; i++) {
    shape->decomp.orbit_size[i] = (i * 2 == n - 1) ? 12 : 24;
    shape->decomp.orbit_offset[i] = offset;
    shape->orbits[i].dim = 1;
    shape->orbits[i].x = i;
    shape->orbits[i].y = 0;
    shape->orbits[i].z = 0;
    offset += shape->decomp.orbit_size[i];
  }

  /* centre orbits have size 24, except the central one when n is odd */
  unsigned int y = 1; unsigned int z = 1;
  for (unsigned int i = num_edge_orbits + 1;
       i < shape->decomp.num_orbits; i++) {
    shape->decomp.orbit_size[i] = 24;
    shape->decomp.orbit_offset[i] = offset;
    shape->orbits[i].dim = 2;
    shape->orbits[i].x = n - 1;
    shape->orbits[i].y = y++;
    shape->orbits[i].z = z;
    offset += shape->decomp.orbit_size[i];

    if (y > num_edge_orbits) {
      z++;
      y = (z > (n - 2) / 2) ? z : 1;
    }
  }
  if (n % 2 == 1) shape->decomp.orbit_size[num_orbits - 1] = 6;
}

void cube_shape_cleanup(cube_shape_t *shape)
{
  free(shape->orbits);
}

void cube_init(puzzle_t *puzzle, cube_t *cube, cube_shape_t *shape)
{
  cube->shape = shape;
  cube->pieces = malloc(cube->shape->decomp.num_pieces * sizeof(uint8_t));

  unsigned int index = 0;
  for (unsigned int i = 0; i < cube->shape->decomp.num_orbits; i++) {
    orbit_t *orbit = &cube->shape->orbits[i];
    for (unsigned int j = 0; j < cube->shape->decomp.orbit_size[i]; j++) {
      cube->pieces[index++] = puzzle->by_stab[orbit->dim][j];
    }
  }
}

static unsigned int act(puzzle_t *puzzle, cube_shape_t *shape,
                        unsigned int i, unsigned int x, unsigned int g)
{
  orbit_t *orbit = &shape->orbits[i];
  unsigned int g0 = puzzle->by_stab[orbit->dim][x];
  unsigned int g1 = group_mul(puzzle->group, g0, g);
  unsigned int j = puzzle->inv_by_stab[orbit->dim][g1];
  printf("act x: %u, g: %u, g0: %u, g1: %u, j: %u\n",
         x, g, g0, g1, j);
  return shape->decomp.orbit_offset[i] +
    j % shape->decomp.orbit_size[i];
}

void cube_cleanup(cube_t *cube)
{
  free(cube->pieces);
}

uint8_t *cube_orbit(cube_t *cube, unsigned int k)
{
  return &cube->pieces[cube->shape->decomp.orbit_offset[k]];
}

static inline int vertex_in_face(unsigned int v, unsigned int f)
{
  return ((v >> (f >> 1)) & 1) != (f & 1);
}

int in_layer(puzzle_t *puzzle, cube_shape_t *shape, orbit_t *orbit,
             unsigned int f, unsigned int l, unsigned int g)
{
  unsigned int f1 =
    decomp_local(&puzzle->decomp,
                 puzzle_act(puzzle,
                            decomp_global(&puzzle->decomp, 2, f),
                            group_inv(puzzle->group, g)));
  switch (orbit->dim) {
  case 0:
    return l == 0 && (f1 & 1);
    break;
  case 1:
    if (l == 0) {
      return f1 == 3 || f1 == 5;
    }
    else if (f1 / 2 == 0) {
      unsigned int x = orbit->x;
      if ((f1 & 1) == 0) x = shape->n - x - 1;
      return l == x;
    }
    break;
  case 2:
    if (l == 0) {
      return f1 == 0;
    }
    else if (f1 / 2 != 0) {
      unsigned int a = f1 / 2;
      unsigned int x = a == 1 ? orbit->y : orbit->z;
      if ((f1 & 1) == 0) x = shape->n - x - 1;
      return l == x;
    }
    break;
  }

  return 0;
}

turn_t *cube_move_(puzzle_t *puzzle, cube_t *conf,
                   unsigned int f, unsigned int l, int c)
{
  return cube_move(puzzle, conf, conf, f, l, c);
}

turn_t *cube_move(puzzle_t *puzzle, cube_t *conf1, cube_t *conf,
                  unsigned int f, unsigned int l, int c)
{
  turn_t *turn = malloc(sizeof(turn_t));
  turn->pieces = malloc(conf->shape->decomp.num_pieces * sizeof(unsigned int));
  turn->num_pieces = 0;

  c = ((c % 4) + 4) % 4;
  unsigned int s = puzzle->by_stab[2][puzzle->decomp.orbit_size[2] * c];
  s = group_conj(puzzle->group, s, puzzle->by_stab[2][f]);

  conf1->shape = conf->shape;
  for (unsigned int k = 0; k < conf->shape->decomp.num_orbits; k++) {
    orbit_t *orbit = &conf->shape->orbits[k];
    for (unsigned int i = 0; i < conf->shape->decomp.orbit_size[k]; i++) {
      unsigned int i0 = conf->shape->decomp.orbit_offset[k] + i;
      if (in_layer(puzzle, conf->shape, orbit, f, l, conf->pieces[i0])) {
        conf1->pieces[i0] = group_mul(puzzle->group, conf->pieces[i0], s);
        turn->pieces[turn->num_pieces++] = i0;
      }
    }
  }

  return turn;
}

void cube_act(puzzle_t *puzzle, cube_t *conf1, cube_t *conf, cube_t *move)
{
  conf1->shape = conf->shape;
  for (unsigned int k = 0; k < conf->shape->decomp.num_orbits; k++) {
    orbit_t *orbit = &conf->shape->orbits[k];
    for (unsigned int i = 0; i < conf->shape->decomp.orbit_size[k]; i++) {
      unsigned int i0 = conf->shape->decomp.orbit_offset[k] + i;
      unsigned int i1 = act(puzzle, conf->shape, k, 0, conf->pieces[i0]);
      printf("%u %u (i0: %u, i1: %u): %u =[ %u ]=> %u\n",
             k, i, i0, i1, conf->pieces[i0],
             move->pieces[i1],
             group_mul(puzzle->group,
                       conf->pieces[i0],
                       move->pieces[i1]));
      conf1->pieces[i0] = group_mul(puzzle->group,
                                    conf->pieces[i0],
                                    move->pieces[i1]);
    }
  }
}

void cube_act_(puzzle_t *syms, cube_t *cube, cube_t *move)
{
  cube_act(syms, cube, cube, move);
}
