#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <stdint.h>

struct abs_poly_t;
typedef struct abs_poly_t abs_poly_t;

struct poly_data_t;
typedef struct poly_data_t poly_data_t;

struct puzzle_t;
typedef struct puzzle_t puzzle_t;

/* [Note]

A configuration is a map from pieces to symmetries. There are two ways
to interpret a configuration: relative and absolute. A relative
configuration maps every piece to the symmetry that needs to be
applied to it to bring it to its current state. An absolute
configuration maps every piece to the symmetry to apply to bring it
from a fixed reference state (that of piece 0) to its current state.

Relative configurations form a group, where the identity is simply the
configuration that maps every piece to the identity symmetry. Absolute
configurations are a torsor over this group.

Relative configurations compose as follows: (ab)(x) = a(x) b(x a(x)),
where symmetries act on pieces on the right. The right action of a
relative configuration a on absolute one u is: (ua)(x) = u(x) a(0
u(x)).
*/
typedef struct
{
  uint8_t *pieces;
} megaminx_t;

/* set the ground absolute configuration */
void megaminx_init(puzzle_t *puzzle, megaminx_t *mm);

void megaminx_cleanup(megaminx_t *mm);

uint8_t *megaminx_orbit(puzzle_t *puzzle, megaminx_t *mm, unsigned int k);

/* action */
void megaminx_act_(puzzle_t *puzzle, megaminx_t *conf, megaminx_t *move);
void megaminx_act(puzzle_t *puzzle, megaminx_t *conf1,
                  megaminx_t *conf, megaminx_t *move);
void megaminx_rotate_(puzzle_t *puzzle, megaminx_t *conf, unsigned int s);
void megaminx_rotate(puzzle_t *puzzle, megaminx_t *conf1,
                     megaminx_t *conf, unsigned int s);

/* generators */
megaminx_t *megaminx_generators(puzzle_t *puzzle,
                                abs_poly_t *dodec,
                                poly_data_t *data,
                                unsigned int *num);

void megaminx_scramble(puzzle_t *puzzle, megaminx_t *mm);

void megaminx_debug(puzzle_t *puzzle, megaminx_t *mm);

#endif /* MEGAMINX_H */
