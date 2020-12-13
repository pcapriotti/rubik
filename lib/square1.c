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

static int in_layer(puzzle_t *puzzle, uint8_t *conf,
                    unsigned int i, unsigned int k, unsigned int g)
{
  if (i < 2) {
    return k < 2 && g % 2 == i;
  }
  else if (i == 2) {
    return k == 2;
  }
  else if (i == 3) {
    unsigned int g0 = conf[decomp_global(puzzle->decomp, 2, 0)];
    g = group_mul(puzzle->group, g,
                  group_inv(puzzle->group, g0 & ~1));
    if (k == 0) return g >= 6 && g < 16;
    if (k == 1 && (g & 1) == 0) return g >= 2 && g <= 12;
    if (k == 1 && (g & 1) == 1) return g >= 9 && g <= 19;
    if (k == 2) return g >> 1 == 6;
  }

  return 0;
}

turn_t *square1_move(puzzle_t *puzzle, uint8_t *conf1, uint8_t *conf,
                     unsigned int f, int c)
{
  turn_t *turn = malloc(sizeof(turn_t));
  turn->num_pieces = 0;
  turn->pieces = malloc(18 * sizeof(unsigned int));

  if (f == 3) {
    /* find position of the middle piece */
    unsigned int p0 = ((conf[decomp_global(puzzle->decomp, 2, 0)] >> 1) + 3) % 12;

    /* make sure that no corner is obstructing */
    /* for (unsigned int i = 0; i < 8; i++) { */
    /*   unsigned int p1 = (conf[decomp_global(puzzle->decomp, 0, i)] >> 1) % 6; */
    /*   unsigned int d = (p0 - p1 + 6) % 6; */
    /*   if (d == 1) return 0; */
    /* } */

    turn->g = group_conj(puzzle->group, 21, 0);
  }
  else if (f < 2) {
    c = (c % 12 + 12) % 12;
    turn->g = c << 1;
  }

  for (unsigned int k = 0; k < puzzle->decomp->num_orbits; k++) {
    for (unsigned int i = 0; i < puzzle->decomp->orbit_size[k]; i++) {
      unsigned int x = decomp_global(puzzle->decomp, k, i);
      if (in_layer(puzzle, conf, f, k, conf[x])) {
        printf("adding piece %u\n", x);
        turn->pieces[turn->num_pieces++] = x;
        conf1[x] = group_mul(puzzle->group, conf[x], turn->g);
      }
    }
  }

  return turn;
}

turn_t *square1_move_(puzzle_t *puzzle, uint8_t *conf, unsigned int f, unsigned int c)
{
  return square1_move(puzzle, conf, conf, f, c);
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

  int i = x >> 1;
  unsigned int s = x & 1;

  int j = y >> 1;
  unsigned int t = y & 1;

  int k = i + (s ? -j : j);
  k = ((k % n) + n) % n;

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

unsigned int square1_facelet(void *data, unsigned int k, unsigned int x, unsigned int i)
{
  static const unsigned int col[] = { 2, 4, 3, 5 };
  if (i == 0) return x & 1;

  if (k < 2) {
    if (x & 1) i = 3 - i;
    x >>= 1;
  }
  else {
    x <<= 1;
  }

  unsigned int index = (x + i - 1) % 4;

  return col[index];
}

void square1_puzzle_cleanup(void *data, puzzle_t *puzzle)
{
}

turn_t *square1_puzzle_move(void *data, uint8_t *conf,
                            unsigned int i, unsigned int l, int c)
{
  puzzle_t *puzzle = data;
  return square1_move_(puzzle, conf, i, c);
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
  puzzle->move_data = puzzle;

  puzzle->scramble = square1_puzzle_scramble;
  puzzle->scramble_data = 0;
}
