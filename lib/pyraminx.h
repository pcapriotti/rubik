#ifndef PYRAMINX_H
#define PYRAMINX_H

#include <stdint.h>

struct puzzle_t;
typedef struct puzzle_t puzzle_t;

struct puzzle_action_t;
typedef struct puzzle_action_t puzzle_action_t;

uint8_t *pyraminx_new(puzzle_action_t *action);

void pyraminx_action_init(puzzle_action_t *action);
void pyraminx_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action);

#endif /* PYRAMINX_H */
