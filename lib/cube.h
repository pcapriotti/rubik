#ifndef CUBE_H
#define CUBE_H

#include <stdint.h>
#include <stddef.h>

#include "puzzle.h"

struct orbit_t {
  int dim;
  int x, y, z;
};
typedef struct orbit_t orbit_t;

typedef struct
{
  unsigned int n;
  orbit_t *orbits;

  decomp_t decomp;
} cube_shape_t;

void cube_shape_init(cube_shape_t *shape, unsigned int n);
void cube_shape_cleanup(cube_shape_t *shape);

struct cube_t
{
  cube_shape_t *shape;
  uint8_t *pieces;
};
typedef struct cube_t cube_t;

uint8_t *cube_new(puzzle_action_t *action, cube_shape_t *shape);

cube_t *cube_generators(cube_t *cube, puzzle_action_t *puzzle, unsigned int *num_gen);
void cube_act(puzzle_action_t *puzzle, cube_t *cube1, cube_t *cube, cube_t *move);
void cube_act_(puzzle_action_t *syms, cube_t *cube, cube_t *move);
turn_t *cube_move_(puzzle_action_t *action, cube_shape_t *shape,
                   uint8_t *conf, unsigned int f, unsigned int l, int c);
turn_t *cube_move(puzzle_action_t *action,
                  cube_shape_t *shape,
                  uint8_t *conf1, uint8_t *conf,
                  unsigned int f, unsigned int l, int c);

void cube_puzzle_init(puzzle_t *puzzle,
                      puzzle_action_t *action,
                      cube_shape_t *shape);

#endif /* CUBE_H */
