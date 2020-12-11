#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <stdint.h>

struct puzzle_action_t;
typedef struct puzzle_action_t puzzle_action_t;

struct puzzle_t;
typedef struct puzzle_t puzzle_t;

struct turn_t;
typedef struct turn_t turn_t;

uint8_t *megaminx_new(puzzle_action_t *action);
void megaminx_scramble(puzzle_action_t *puzzle, uint8_t *mm);

turn_t *megaminx_move_(puzzle_action_t *action, uint8_t *conf,
                       unsigned int f, int c);
turn_t *megaminx_move(puzzle_action_t *action, uint8_t *conf1, uint8_t *conf,
                      unsigned int f, int c);

void megaminx_puzzle_init(puzzle_t *puzzle, puzzle_action_t *action);

#endif /* MEGAMINX_H */
