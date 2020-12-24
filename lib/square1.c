#include "square1.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "group.h"
#include "puzzle.h"
#include "perm.h"

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

static int middle_layer_sym(unsigned int k, unsigned int g)
{
  if (k == 0) return g >= 6 && g < 16;
  if (k == 1 && (g & 1) == 0) return g >= 2 && g <= 12;
  if (k == 1 && (g & 1) == 1) return g >= 9 && g <= 19;
  if (k == 2) return g == 12 || g == 9;
  return 0;
}

static int in_layer(puzzle_t *puzzle,
                    unsigned int i, unsigned int k,
                    int c, unsigned int g)
{
  if (i < 2) {
    return k < 2 && g % 2 == i;
  }
  else if (i == 2) {
    return k == 2;
  }
  else if (i == 3) {
    return middle_layer_sym(k, g) ^ (c > 0);
  }

  return 0;
}

turn_t *square1_move(puzzle_t *puzzle, uint8_t *conf1, uint8_t *conf,
                     unsigned int f, int c)
{
  unsigned int sym;

  /* find rotation of the middle piece */
  unsigned int g0 = conf[decomp_global(puzzle->decomp, 2, 0)];
  if (g0 & 1) g0 = group_mul(puzzle->group, g0, 21);

  if (f == 3) {
    /* position of the middle piece */
    unsigned int p0 = (g0 + 3) % 12;

    /* make sure that no corner is obstructing */
    for (unsigned int i = 0; i < 8; i++) {
      unsigned int p1 = (conf[decomp_global(puzzle->decomp, 0, i)] >> 1) % 6;
      unsigned int d = (p0 - p1 + 6) % 6;
      if (d == 1) return 0;
    }

    sym = group_conj(puzzle->group, 21, 0);
  }
  else if (f < 2) {
    if (f == 1) c = -c;
    c = (c % 12 + 12) % 12;
    sym = c << 1;
  }
  else {
    return 0;
  }

  turn_t *turn = malloc(sizeof(turn_t));
  turn->num_pieces = 0;
  turn->pieces = malloc(18 * sizeof(unsigned int));
  turn->g = sym;

  for (unsigned int k = 0; k < puzzle->decomp->num_orbits; k++) {
    for (unsigned int i = 0; i < puzzle->decomp->orbit_size[k]; i++) {
      unsigned int x = decomp_global(puzzle->decomp, k, i);
      unsigned int g = group_mul(puzzle->group, conf[x],
                                 group_inv(puzzle->group, g0 & ~1));
      if (in_layer(puzzle, f, k, c, g)) {
        turn->pieces[turn->num_pieces++] = x;
        conf1[x] = group_mul(puzzle->group, conf[x], turn->g);
      }
    }
  }

  printf("conf: ");
  debug_perm(conf, 18);
  printf("\n");

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

  int k = (t ? -i : i) + j;
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

void square1_perm(uint8_t *perm_inv)
{
  perm_id(perm_inv, 16);
  shuffle(perm_inv, 16);

  /* make sure that the two layers are complete and divisible in half */
  unsigned int count = 0;
  int e = -1;
  for (unsigned int i = 0; i < 16; i++) {
    /* remember position of the last edge */
    if (perm_inv[i] >= 8) {
      e = i;
      count += 1;
    }
    /* check if a corner is placed where there only an edge would fit */
    else if (count % 6 == 5 && perm_inv[i] < 8) {
      /* for parity reasons, we must have encountered at least one edge */
      assert(e >= 0);

      /* swap with that edge */
      uint8_t c = perm_inv[i];
      perm_inv[i] = perm_inv[e];
      perm_inv[e] = c;

      count += 2;
    }
    else {
      count += 2;
    }
  }
}

void square1_scramble(puzzle_t *puzzle, uint8_t *conf)
{
  uint8_t perm_inv[16];
  square1_perm(perm_inv);

  unsigned int count = 0;
  unsigned int offset = rand() % 2;
  unsigned int l = 0;
  for (unsigned int i = 0; i < 16; i++) {
    unsigned int j = perm_inv[i];

    unsigned int k = j >= 8;
    unsigned int x = j % 8;

    unsigned int pos = (offset + (k ? count + (l ? 1 : -2) : count)) % 12;
    unsigned int sym = (pos << 1) | l;

    conf[decomp_global(puzzle->decomp, k, x)] = sym;
    count += k ? 1 : 2;

    if (count >= 12) {
      offset = rand() % 2;
      count = 0;
      l++;
    }
  }

  unsigned int x = rand() % 8;
  unsigned int flip0 = x & 1;
  unsigned int flip1 = (x >> 1) & 1;
  unsigned int pos0 = flip0 ? 21 : 0;
  unsigned int pos1 = flip1 ? 9 : 12;

  conf[decomp_global(puzzle->decomp, 2, 0)] = (x >> 2) ? pos0 : pos1;
  conf[decomp_global(puzzle->decomp, 2, 1)] = (x >> 2) ? pos1 : pos0;

  printf("scramble conf: ");
  debug_perm(conf, 18);
  printf("\n");
}

void square1_puzzle_cleanup(void *data, puzzle_t *puzzle)
{
  free(puzzle->decomp);
}

turn_t *square1_puzzle_move(void *data, uint8_t *conf,
                            unsigned int i, unsigned int l, int c)
{
  puzzle_t *puzzle = data;
  return square1_move_(puzzle, conf, i, c);
}

void square1_puzzle_scramble(void *data, uint8_t *conf)
{
  printf("scrambling\n");
  puzzle_t *puzzle = data;
  square1_scramble(puzzle, conf);
}

void square1_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action)
{
  puzzle->group = action->group;
  puzzle->decomp = malloc(sizeof(decomp_t));
  unsigned int orbit_size[] = { 8, 8, 2 };
  decomp_init(puzzle->decomp, 3, orbit_size);

  puzzle->facelet = square1_facelet;
  puzzle->facelet_data = 0;

  puzzle->cleanup = square1_puzzle_cleanup;
  puzzle->cleanup_data = 0;

  puzzle->move = square1_puzzle_move;
  puzzle->move_data = puzzle;

  puzzle->scramble = square1_puzzle_scramble;
  puzzle->scramble_data = puzzle;
}
