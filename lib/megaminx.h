#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <stdint.h>

struct abs_poly_t;
typedef struct abs_poly_t abs_poly_t;

static const unsigned int megaminx_num_syms = 60;

#define MEGAMINX_NUM_CORNERS 20
#define MEGAMINX_NUM_EDGES 30
#define MEGAMINX_NUM_CENTRES 12
#define MEGAMINX_NUM_PIECES \
  (MEGAMINX_NUM_CORNERS + MEGAMINX_NUM_EDGES + MEGAMINX_NUM_CENTRES)

typedef struct {
  unsigned int *by_vertex;
  unsigned int *by_edge;

  uint32_t *face_action;
  uint8_t *vertex_action;
  uint8_t *edge_action;

  unsigned int *edges_by_face;

  /* multiplication table */
  uint8_t *mul;
  /* inverse multiplication table */
  uint8_t *inv_mul;
} symmetries_t;

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
  uint8_t pieces[MEGAMINX_NUM_PIECES];
} megaminx_t;

uint8_t *megaminx_corner(megaminx_t *mm, unsigned int c);
uint8_t *megaminx_edge(megaminx_t *mm, unsigned int e);
uint8_t *megaminx_centre(megaminx_t *mm, unsigned int f);
unsigned int megaminx_orientations(unsigned int x);

/* set the ground absolute configuration */
void megaminx_init(symmetries_t *syms, megaminx_t *mm);

/* action */
void megaminx_act_(symmetries_t *syms, megaminx_t *conf, megaminx_t *move);
void megaminx_act(symmetries_t *syms, megaminx_t *conf1,
                  megaminx_t *conf, megaminx_t *move);
void megaminx_rotate_(symmetries_t *syms, megaminx_t *conf, unsigned int s);
void megaminx_rotate(symmetries_t *syms, megaminx_t *conf1,
                     megaminx_t *conf, unsigned int s);

/* generators */
megaminx_t *megaminx_generators(symmetries_t *syms,
                                abs_poly_t *dodec,
                                unsigned int *num);

void megaminx_scramble(symmetries_t *syms, megaminx_t *mm);

void megaminx_debug(symmetries_t *syms, megaminx_t *mm);

#endif /* MEGAMINX_H */
