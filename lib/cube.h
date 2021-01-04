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

void cube_orbit_act_(unsigned int n, orbit_t *orbit, unsigned int g);

typedef struct
{
  unsigned int n;
  orbit_t *orbits;

  decomp_t decomp;
} cube_shape_t;

void cube_shape_init(cube_shape_t *shape, unsigned int n);
void cube_shape_cleanup(cube_shape_t *shape);

struct cube_puzzle_data_t
{
  puzzle_action_t *action;
  cube_shape_t *shape;
  puzzle_t *puzzle;
};
typedef struct cube_puzzle_data_t cube_puzzle_data_t;

uint8_t *cube_new(puzzle_action_t *action, cube_shape_t *shape);

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
