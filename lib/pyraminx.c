#include "pyraminx.h"

#include "group.h"
#include "perm.h"
#include "puzzle.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/* The symmetry group of the pyraminx is A_4, which is also the
symmetry group of a tetrahedron. */
void a4_init(group_t *group)
{
  static const unsigned int num_syms = 12;
  uint8_t *table = malloc(num_syms * num_syms);

  unsigned int i = 0;
  for (unsigned int index = 0; index < 24; index++) {
    uint8_t lehmer[4], perm[4];
    lehmer_from_index(lehmer, 4, index, 4);
    if (lehmer_sign(lehmer, 4)) continue;

    perm_from_lehmer(perm, lehmer, 4);

    unsigned int j = 0;
    for (unsigned int index1 = 0; index1 < 24; index1++) {
      uint8_t lehmer1[4], perm1[4];
      lehmer_from_index(lehmer1, 4, index1, 4);
      if (lehmer_sign(lehmer1, 4)) continue;

      perm_from_lehmer(perm1, lehmer1, 4);
      perm_lmul(perm1, perm, 4);

      table[j + i * num_syms] = perm_index(perm1, 4, 4) / 2;
      j++;
    }

    i++;
  }

  group_from_table(group, num_syms, table);
}

void pyraminx_action_init(puzzle_action_t *action)
{
  group_t *group = malloc(sizeof(group_t));
  a4_init(group);
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

  /* orbit[1][0] = 0; */
  /* orbit[1][1] = 1; */
  /* orbit[1][2] = 0; */
  /* orbit[1][3] = 0; */
  /* orbit[1][4] = 0; */
  /* orbit[1][5] = 0; */

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
  puzzle->scramble_data = puzzle;
}
