#ifndef SQUARE1_H
#define SQUARE1_H

#include <stdint.h>

struct decomp_t;
typedef struct decomp_t decomp_t;

struct puzzle_action_t;
typedef struct puzzle_action_t puzzle_action_t;

struct puzzle_t;
typedef struct puzzle_t puzzle_t;

struct turn_t;
typedef struct turn_t turn_t;

uint8_t *square1_new(decomp_t *decomp);
void square1_action_init(puzzle_action_t *action);
void square1_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action);

#endif /* SQUARE1_H */
