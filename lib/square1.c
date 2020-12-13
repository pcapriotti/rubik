#include "square1.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "group.h"
#include "puzzle.h"

uint8_t *square1_new(decomp_t *decomp)
{
  uint8_t *conf = malloc(decomp->num_pieces);

  for (unsigned int i = 0; i < 8; i++) {
    unsigned int sym = ((i >> 1) * 6) | (i & 1);
    conf[decomp_global(decomp, 0, i)] = sym;
    conf[decomp_global(decomp, 1, i)] = sym;
  }
  for (unsigned int i = 0; i < 2; i++) {
    conf[decomp_global(decomp, 2, i)] = i * 12;
  }

  return conf;
}

void square1_rotate(group_t *group,
                    uint8_t *conf1, uint8_t *conf,
                    unsigned int f, int c)
{
  c = (c % 12 + 12) % 12;
  unsigned int sym = c << 1;

  assert(f < 3);

  for (unsigned int i = 0; i < 12; i++) {
    unsigned int x = i + f * 12;
    conf1[x] = group_mul(group, conf[x], sym);
  }
}

void square1_rotate_(group_t *group, uint8_t *conf, unsigned int f, int c)
{
  square1_rotate(group, conf, conf, f, c);
}

/* The symmetry group of the square 1 is simply the dihedral group
D_12. The group acts on the possible positions of pieces, which are 12
for each of the three layers. This puzzle is somewhat different from a
cube or the megaminx, because there are fewer pieces than positions,
and the symmetry group does not preserve the geometry of the puzzle.

The rotation of D_12 simply cycles the positions of every layer, while
the symmetries swap the top and bottom layer, and reflect the pieces
across a horizontal axis. There are two orbits: top/bottom and centre.
*/

unsigned int dihedral_group_mul(void *data_, unsigned int x, unsigned int y)
{
  unsigned int *data = data_;
  unsigned int n = *data;

  unsigned int i = x >> 1;
  unsigned int s = x & 1;

  unsigned int j = y >> 1;
  unsigned int t = y & 1;

  unsigned int k = i + (s ? j : -j);
  k = (k % n + n) % n;
  unsigned int u = s ^ t;

  return (k << 1) | u;
}

void square1_action_init(puzzle_action_t *action)
{
  static const unsigned int num_syms = 24;

  group_t dihedral;
  {
    dihedral.num = num_syms;
    dihedral.cleanup = free;
    unsigned int *data = malloc(sizeof(unsigned int));
    *data = 12;
    dihedral.data = data;
    dihedral.mul = dihedral_group_mul;
    dihedral.inv_mul = 0;
  }

  group_t *group = malloc(sizeof(group_t));
  group_memo(group, &dihedral);

  unsigned int num_orbits = 2;
  unsigned int orbit_size[] = { 24, 12 };
  uint8_t stab_gen[] = { 0, 1 };

  uint8_t *orbit[2];
  uint8_t *stab[2];

  for (unsigned int k = 0; k < num_orbits; k++) {
    orbit[k] = malloc(orbit_size[k]);
    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* top / bottom */
  for (unsigned int v = 0; v < 24; v++) {
    orbit[0][v] = v;
  }

  /* middle */
  for (unsigned int v = 0; v < 12; v++) {
    orbit[1][v] = v << 1;
  }

  puzzle_action_init(action, num_orbits, orbit_size,
                     group, orbit, stab);

  for (unsigned int k = 0; k < num_orbits; k++) {
    free(orbit[k]);
    free(stab[k]);
  }
}

unsigned int square1_facelet(void *data, unsigned int k, unsigned int x, unsigned int g)
{
  return 1;
}

void square1_puzzle_cleanup(void *data, puzzle_t *puzzle)
{
}

turn_t *square1_puzzle_move(void *data, uint8_t *conf,
                            unsigned int f, unsigned int l, int c)
{
  return 0;
}

void square1_puzzle_scramble(void *data, uint8_t *conf)
{
}

void square1_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action)
{
  puzzle->group = action->group;
  puzzle->decomp = malloc(sizeof(decomp_t));
  unsigned int orbit_size[] = { 8, 8, 2 };
  decomp_init(puzzle->decomp, 3, orbit_size);
  puzzle->num_faces = 6;

  puzzle->orbit = puzzle_orbit_default;
  puzzle->orbit_data = 0;

  puzzle->facelet = square1_facelet;
  puzzle->facelet_data = 0;

  puzzle->cleanup = square1_puzzle_cleanup;
  puzzle->cleanup_data = 0;

  puzzle->move = square1_puzzle_move;
  puzzle->move_data = 0;

  puzzle->scramble = square1_puzzle_scramble;
  puzzle->scramble_data = 0;
}
