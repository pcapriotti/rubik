#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdint.h>

struct group_t;
typedef struct group_t group_t;

struct action_t;
typedef struct action_t action_t;

struct symmetries_t {
  unsigned int num;

  unsigned int num_vertices;
  unsigned int num_edges;
  unsigned int num_faces;

  unsigned int *by_vertex;
  unsigned int *by_edge;
  unsigned int *by_face;

  uint8_t *face_action;
  uint8_t *vertex_action;
  uint8_t *edge_action;

  unsigned int *edges_by_face;

  /* multiplication table */
  uint8_t *mul;
  /* inverse multiplication table */
  uint8_t *inv_mul;
};
typedef struct symmetries_t symmetries_t;

unsigned int symmetries_by_cell(symmetries_t *syms,
                                unsigned int dim,
                                unsigned int i);

/* Orbit decomposition */
struct decomp_t
{
  /* The cardinality of the set X of pieces. */
  unsigned int num_pieces;

  /* We assume a choice of a representative for every orbit, and that
     the pieces are enumerated by orbit, so that the representative
     has index 0 in its orbit.

     Orbit i will be denoted by X_i, and its representative by r_i.
  */
  unsigned int num_orbits;
  unsigned int *orbit_size;
  unsigned int *orbit_offset;
};
typedef struct decomp_t decomp_t;

struct puzzle_action_t
{
  /* Symmetry group G of the puzzle. */
  group_t *group;

  /* Symmetries by stabiliser coset.

     This is a 3-dimensional matrix corresponding to the choice of a
     symmetry g(x) for each element x, such that g(x) maps the
     representative of the orbit of x to x itself.

     Such a choice determines, for each orbit i, a set isomorphism
     phi_i between the product Stab(r_i) x X_i and G itself.

     At index (i, j, k) the matrix contains phi_i(u_j, x_k), where x_k
     is the k-th element of X_i, and u_j is the j-th element of
     Stab(r_i) in some enumeration.
   */
  uint8_t **by_stab;

  /* The inverse of the above isomorphism family. */
  uint8_t **inv_by_stab;

  decomp_t decomp;
};
typedef struct puzzle_action_t puzzle_action_t;

unsigned int decomp_orbit_of(decomp_t *puzzle, unsigned int x);
unsigned int decomp_global(decomp_t *puzzle, unsigned int i, unsigned int j);
unsigned int decomp_local(decomp_t *puzzle, unsigned int j);
unsigned int decomp_repr(decomp_t *puzzle, unsigned int i);

void decomp_cleanup(decomp_t *decomp);
void decomp_init(decomp_t *decomp,
                 unsigned int num_orbits,
                 unsigned int *orbit_size);


void puzzle_action_init(puzzle_action_t *puzzle,
                               unsigned int num_orbits, unsigned int *orbit_size,
                               group_t *group, uint8_t **orbit, uint8_t **stab);
void puzzle_action_cleanup(puzzle_action_t *puzzle);
unsigned int puzzle_action_act(puzzle_action_t *puzzle, unsigned int x, unsigned int g);

struct turn_t
{
  unsigned int g;
  unsigned int num_pieces;
  unsigned int *pieces;
};
typedef struct turn_t turn_t;

void turn_del(turn_t *turn);

void decomp_split_turn(decomp_t *decomp, turn_t *turn,
                       unsigned int *num_pieces,
                       unsigned int **splits);

#endif /* PUZZLE_H */
