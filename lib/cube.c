#include "cube.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "group.h"
#include "puzzle.h"
#include "utils.h"
#include "perm.h"

/* [Note]

A configuration is a map from pieces to symmetries. There are two ways
to interpret a configuration: relative and absolute. A relative
configuration maps every piece to the symmetry that needs to be
applied to it to bring it to its current state. An absolute
configuration maps every piece to the symmetry to apply to bring it
from a fixed reference state (that of piece 0) to its current state.

Relative configurations form a group, where the identity is simply the
configuration that maps every piece to the identity symmetry. Absolute
configurations are a torsor over this group.

Relative configurations compose as follows: (ab)(x) = a(x) b(x a(x)),
where symmetries act on pieces on the right. The right action of a
relative configuration a on absolute one u is: (ua)(x) = u(x) a(0
u(x)).
*/

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

uint8_t *cube_new(puzzle_action_t *action, cube_shape_t *shape)
{
  uint8_t *conf = malloc(shape->decomp.num_pieces * sizeof(uint8_t));

  unsigned int index = 0;
  for (unsigned int i = 0; i < shape->decomp.num_orbits; i++) {
    orbit_t *orbit = &shape->orbits[i];
    for (unsigned int j = 0; j < shape->decomp.orbit_size[i]; j++) {
      conf[index++] = action->by_stab[orbit->dim][j];
    }
  }

  return conf;
}

int in_layer(puzzle_action_t *action, cube_shape_t *shape, orbit_t *orbit,
             unsigned int f, unsigned int l, unsigned int g)
{
  unsigned int f1 =
    decomp_local(&action->decomp,
                 puzzle_action_act(action,
                            decomp_global(&action->decomp, 2, f),
                            group_inv(action->group, g)));
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

turn_t *cube_move_(puzzle_action_t *action, cube_shape_t *shape,
                   uint8_t *conf, unsigned int f, unsigned int l, int c)
{
  return cube_move(action, shape, conf, conf, f, l, c);
}

turn_t *cube_move(puzzle_action_t *action,
                  cube_shape_t *shape,
                  uint8_t *conf1, uint8_t *conf,
                  unsigned int f, unsigned int l, int c)
{
  turn_t *turn = malloc(sizeof(turn_t));
  turn->pieces = malloc(shape->decomp.num_pieces * sizeof(unsigned int));
  turn->num_pieces = 0;

  turn->g = puzzle_action_stab(action, 2, f, c);

  for (unsigned int k = 0; k < shape->decomp.num_orbits; k++) {
    orbit_t *orbit = &shape->orbits[k];
    for (unsigned int i = 0; i < shape->decomp.orbit_size[k]; i++) {
      unsigned int i0 = shape->decomp.orbit_offset[k] + i;
      if (in_layer(action, shape, orbit, f, l, conf[i0])) {
        conf1[i0] = group_mul(action->group, conf[i0], turn->g);
        turn->pieces[turn->num_pieces++] = i0;
      }
    }
  }

  return turn;
}

void cube_scramble(puzzle_action_t *action, cube_shape_t *shape, uint8_t *conf)
{
  uint8_t parity = 0;
  for (int k = shape->decomp.num_orbits - 1; k >= 0; k--) {
    unsigned int orb_size = shape->decomp.orbit_size[k];
    unsigned int stab_size = action->group->num / orb_size;
    uint8_t *perm = malloc(orb_size);
    perm_id(perm, orb_size);

    if (k == 0 && shape->n % 2 == 1) {
      /* only even permutations on odd cubes */
      parity_shuffle(perm, orb_size, parity);
    }
    else {
      shuffle(perm, orb_size);

      /* keep track of corner and edge parity */
      if (shape->orbits[k].dim < 2)
        parity ^= perm_sign(perm, orb_size);
    }

    unsigned int total = 0;
    for (unsigned int i = 0; i < orb_size; i++) {
      unsigned int o;
      if (i == orb_size - 1) {
        o = (stab_size - total % stab_size) % stab_size;
        assert((total + o) % stab_size == 0);
      }
      else {
        o = rand() % stab_size;
        total += o;
      }

      unsigned int x = decomp_global(&shape->decomp, k, i);
      unsigned int sym = action->by_stab[k][o * orb_size + perm[i]];
      printf("setting %u (orb: %u, local: %u) to sym %u\n",
             x, k, i, sym);
      conf[x] = sym;
    }

    free(perm);
  }
}

struct cube_puzzle_move_data_t
{
  puzzle_action_t *action;
  cube_shape_t *shape;
};

void *cube_puzzle_orbit(void *data, unsigned int i)
{
  cube_shape_t *shape = data;
  return &shape->orbits[i];
}

void cube_puzzle_cleanup(void *data, puzzle_t *puzzle)
{
  cube_shape_t *shape = puzzle->orbit_data;
  cube_shape_cleanup(shape);
  free(shape);

  puzzle_action_t *action = puzzle->face_action_data;
  puzzle_action_cleanup(action);
  free(action);

  free(puzzle->move_data);
}

turn_t *cube_puzzle_move(void *data_, uint8_t *conf,
                         unsigned int f, unsigned int l, int c)
{
  struct cube_puzzle_move_data_t *data = data_;
  return cube_move_(data->action, data->shape, conf, f, l, c);
}

void cube_puzzle_scramble(void *data_, uint8_t *conf)
{
  struct cube_puzzle_move_data_t *data = data_;
  cube_scramble(data->action, data->shape, conf);
}

void cube_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action, cube_shape_t *shape)
{
  puzzle->group = action->group;
  puzzle->decomp = &shape->decomp;
  puzzle->num_faces = 6;

  puzzle->orbit = cube_puzzle_orbit;
  puzzle->orbit_data = shape;

  puzzle->face_action = puzzle_face_action_default;
  puzzle->face_action_data = action;

  puzzle->cleanup = cube_puzzle_cleanup;
  puzzle->cleanup_data = 0;

  puzzle->move = cube_puzzle_move;
  struct cube_puzzle_move_data_t *mdata =
    malloc(sizeof(struct cube_puzzle_move_data_t));
  mdata->action = action;
  mdata->shape = shape;
  puzzle->move_data = mdata;

  puzzle->scramble = cube_puzzle_scramble;
  puzzle->scramble_data = mdata;
}
