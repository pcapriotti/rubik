#include "pyraminx.h"

#include "group.h"
#include "perm.h"
#include "puzzle.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/* The symmetry group of the pyraminx is A_4, which is also the
symmetry group of a tetrahedron. */

void pyraminx_action_init(puzzle_action_t *action)
{
  group_t *group = malloc(sizeof(group_t));
  group_a4_init(group);
  unsigned int num_syms = group->num;

  unsigned int num_orbits = 3;
  unsigned int orbit_size[] = { 4, 6, 12 };
  uint8_t stab_gen[] = { 1, 3, 0 };

  uint8_t *orbit[3];
  uint8_t *stab[3];

  for (unsigned int k = 0; k < num_orbits; k++) {
    orbit[k] = malloc(orbit_size[k]);
    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* the corner orbit corresponds to the Klein subgroup of A_4 */
  uint8_t klein[4] = {0, 3, 8, 11};
  memcpy(orbit[0], klein, 4);

  /* for an edge x < y, take the unique symmetry that maps 0 */
  /* to x and 1 to y */
  unsigned int index = 0;
  for (unsigned int x = 0; x < 3; x++) {
    for (unsigned int y = x + 1; y < 4; y++) {
      printf("x: %u, y: %u\n", x, y);
      uint8_t lehmer[4];
      lehmer[0] = x;
      lehmer[1] = y - 1;
      lehmer[2] = lehmer[3] = 0;
      lehmer[2] = lehmer_sign(lehmer, 4);

      orbit[1][index++] = lehmer_index(lehmer, 4, 4) / 2;
      printf("lehmer: ");
      debug_perm(lehmer, 4);

      uint8_t perm[4];
      perm_from_lehmer(perm, lehmer, 4);
      printf("perm: ");
      debug_perm(perm, 4);

      printf(" orbit[1][%u]: %u\n", index - 1, orbit[1][index - 1]);
    }
  }

  /* centres are just identified with symmetries */
  for (unsigned int i = 0; i < num_syms; i++) {
    orbit[2][i] = i;
  }

  puzzle_action_init(action, num_orbits, orbit_size,
                     group, orbit, stab);

  for (unsigned int k = 0; k < num_orbits; k++) {
    free(orbit[k]);
    free(stab[k]);
  }
}

uint8_t *pyraminx_new(puzzle_action_t *action)
{
  uint8_t *conf = malloc(action->decomp.num_pieces * sizeof(uint8_t));
  unsigned int index = 0;
  for (unsigned int i = 0; i < action->decomp.num_orbits; i++) {
    for (unsigned int j = 0; j < action->decomp.orbit_size[i]; j++) {
      conf[index++] = action->by_stab[i][j];
    }
  }

  return conf;
}

void pyraminx_puzzle_cleanup(void *data, struct puzzle_t *puzzle)
{
}

static int in_layer(puzzle_action_t *action,
                    unsigned int k, unsigned int v,
                    unsigned int l, unsigned int g)
{
  v = puzzle_action_local_act(action, 0, v,
                              group_inv(action->group, g)) % 4;

  if (l == 0) {
    return k == 0 && v == 0;
  }

  int ret;
  switch (k) {
  case 0:
  case 2:
    ret = v == 0;
    break;
  case 1:
    ret = v == 0 || v == 1;
    break;
  }

  return l == 1 ? ret : !ret;
}

turn_t *pyraminx_move(puzzle_action_t *action,
                      uint8_t *conf1, uint8_t *conf,
                      unsigned int v, unsigned int l, int c)
{
  turn_t *turn = malloc(sizeof(turn_t));
  turn->pieces = malloc(action->decomp.num_pieces * sizeof(unsigned int));
  turn->num_pieces = 0;

  turn->g = puzzle_action_stab(action, 0, v, c);

  for (unsigned int k = 0; k < action->decomp.num_orbits; k++) {
    for (unsigned int i = 0; i < action->decomp.orbit_size[k]; i++) {
      unsigned int i0 = action->decomp.orbit_offset[k] + i;
      if (in_layer(action, k, v, l, conf[i0])) {
        conf1[i0] = group_mul(action->group, conf[i0], turn->g);
        turn->pieces[turn->num_pieces++] = i0;
      }
    }
  }

  return turn;
}

turn_t *pyraminx_move_(puzzle_action_t *action, uint8_t *conf,
                       unsigned int v, unsigned int l, int c)
{
  return pyraminx_move(action, conf, conf, v, l, c);
}

turn_t *pyraminx_puzzle_move(void *data, uint8_t *conf,
                             unsigned int v, unsigned int l, int c)
{
  puzzle_action_t *action = data;
  return pyraminx_move_(action, conf, v, l, c);
}

void pyraminx_puzzle_scramble(void *data, uint8_t *conf)
{
  unsigned int vo = rand() % 81;

  unsigned int co = rand() % 81;

  uint8_t edges[6];
  perm_id(edges, 6);
  parity_shuffle(edges, 6, 0);

  unsigned int eo = rand() & 0x1f;
  eo |= ((__builtin_popcount(eo) & 1) << 5);


  puzzle_action_t *action = data;

  /* vertices */
  for (unsigned int i = 0; i < 4; i++) {
    unsigned int o = vo % 3;
    unsigned int sym = action->by_stab[0][i + o * 4];
    conf[decomp_global(&action->decomp, 0, i)] = sym;
    vo /= 3;
  }

  /* centres */
  unsigned int cos[4];
  for (unsigned int v = 0; v < 4; v++) {
    cos[v] = puzzle_action_stab(action, 0, v, co % 3);
  }
  for (unsigned int g = 0; g < 12; g++) {
    unsigned int v = puzzle_action_local_act(action, 0, 0, g);
    unsigned int g1 = group_mul(action->group, g, cos[v]);
    conf[decomp_global(&action->decomp, 2, g)] = g1;
  }

  /* edges */
  for (unsigned int e = 0; e < 6; e++) {
    unsigned int o = (eo >> e) & 1;
    unsigned int g = action->by_stab[1][edges[e] + o * 6];
    conf[decomp_global(&action->decomp, 1, e)] = g;
  }
}

unsigned int pyraminx_facelet(void *data, unsigned int k,
                              unsigned int x, unsigned int i)
{
  puzzle_action_t *action = data;
  unsigned int g = action->by_stab[k][x];
  unsigned int x0 = decomp_global(&action->decomp, 0, i);
  unsigned int y0 = puzzle_action_act(action, x0, g);
  return decomp_local(&action->decomp, y0);
}

void pyraminx_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action)
{
  puzzle->group = action->group;
  puzzle->decomp = &action->decomp;
  puzzle->num_faces = 4;

  puzzle->orbit = puzzle_orbit_default;
  puzzle->orbit_data = 0;

  puzzle->facelet = pyraminx_facelet;
  puzzle->facelet_data = action;

  puzzle->cleanup = pyraminx_puzzle_cleanup;
  puzzle->cleanup_data = 0;

  puzzle->move = pyraminx_puzzle_move;
  puzzle->move_data = action;

  puzzle->scramble = pyraminx_puzzle_scramble;
  puzzle->scramble_data = action;
}
