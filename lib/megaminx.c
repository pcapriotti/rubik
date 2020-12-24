#include "megaminx.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abs_poly.h"
#include "group.h"
#include "perm.h"
#include "puzzle.h"
#include "utils.h"

uint8_t *megaminx_new(puzzle_action_t *action)
{
  uint8_t *conf = malloc(action->decomp.num_pieces);
  unsigned int index = 0;
  for (unsigned int k = 0; k < action->decomp.num_orbits; k++) {
    for (unsigned int i = 0; i < action->decomp.orbit_size[k]; i++) {
      conf[index++] = action->by_stab[k][i];
    }
  }
  return conf;
}

static int in_layer(puzzle_action_t *action, unsigned int k, unsigned int f, unsigned int g)
{
  unsigned int f1 =
    decomp_local(&action->decomp,
                 puzzle_action_act(action,
                            decomp_global(&action->decomp, 2, f),
                            group_inv(action->group, g)));
  switch (k) {
  case 0:
    return f1 == 0 || f1 == 2 || f1 == 10;
    break;
  case 1:
    return f1 == 0 || f1 == 2;
    break;
  case 2:
    return f1 == 0;
    break;
  }

  return 0;
}

turn_t *megaminx_move_(puzzle_action_t *action, uint8_t *conf,
                       unsigned int f, int c)
{
  return megaminx_move(action, conf, conf, f, c);
}

turn_t *megaminx_move(puzzle_action_t *action, uint8_t *conf1, uint8_t *conf,
                      unsigned int f, int c)
{
  turn_t *turn = malloc(sizeof(turn_t));
  turn->pieces = malloc(action->decomp.num_pieces * sizeof(unsigned int));
  turn->num_pieces = 0;

  turn->g = puzzle_action_stab(action, 2, f, c);

  for (unsigned int k = 0; k < action->decomp.num_orbits; k++) {
    for (unsigned int i = 0; i < action->decomp.orbit_size[k]; i++) {
      unsigned int i0 = action->decomp.orbit_offset[k] + i;
      if (in_layer(action, k, f, conf[i0])) {
        conf1[i0] = group_mul(action->group, conf[i0], turn->g);
        turn->pieces[turn->num_pieces++] = i0;
      }
    }
  }

  return turn;
}

void megaminx_scramble(puzzle_action_t *action, uint8_t *conf)
{
  for (unsigned int k = 0; k < 2; k++) {
    unsigned int orb_size = action->decomp.orbit_size[k];
    unsigned int stab_size = action->group->num / orb_size;
    uint8_t *perm = malloc(orb_size);
    perm_id(perm, orb_size);
    parity_shuffle(perm, orb_size, 0);

    unsigned int total = 0;
    for (unsigned int i = 0; i < orb_size; i++) {
      unsigned int o;
      if (i == orb_size - 1) {
        o = (stab_size - total % stab_size) % stab_size;
      }
      else {
        o = rand() % stab_size;
        total += o;
      }
      conf[decomp_global(&action->decomp, k, i)] =
        action->by_stab[k][o * orb_size + perm[i]];
    }

    free(perm);
  }
}

static void megaminx_puzzle_cleanup(void *data, puzzle_t *puzzle)
{
  puzzle_action_t *action = puzzle->facelet_data;
  puzzle_action_cleanup(action);
  free(action);
}


turn_t *megaminx_puzzle_move(void *data, uint8_t *conf,
                             unsigned int f, unsigned int l, int c)
{
  puzzle_action_t *action = data;
  if (l != 0) return 0;
  return megaminx_move_(action, conf, f, c);
}

void megaminx_puzzle_scramble(void *data, uint8_t *conf)
{
  puzzle_action_t *action = data;
  megaminx_scramble(action, conf);
}

void megaminx_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action)
{
  puzzle->group = action->group;
  puzzle->decomp = &action->decomp;

  puzzle->facelet = puzzle_facelet_default;
  puzzle->facelet_data = action;

  puzzle->cleanup = megaminx_puzzle_cleanup;
  puzzle->cleanup_data = 0;

  puzzle->move = megaminx_puzzle_move;
  puzzle->move_data = action;

  puzzle->scramble = megaminx_puzzle_scramble;
  puzzle->scramble_data = action;
}
