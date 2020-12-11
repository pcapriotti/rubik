#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <stdint.h>

struct abs_poly_t;
typedef struct abs_poly_t abs_poly_t;

struct poly_data_t;
typedef struct poly_data_t poly_data_t;

struct puzzle_action_t;
typedef struct puzzle_action_t puzzle_action_t;

struct puzzle_t;
typedef struct puzzle_t puzzle_t;

uint8_t *megaminx_new(puzzle_action_t *action);
void megaminx_scramble(puzzle_action_t *puzzle, uint8_t *mm);

void megaminx_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action);

#endif /* MEGAMINX_H */
