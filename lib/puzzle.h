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

struct puzzle_t
{
  /* The cardinality of the set X of pieces. */
  unsigned int num_pieces;

  /* Symmetry group G of the puzzle. */
  group_t *group;

  /* Action of G on the pieces. */
  action_t *action;

  /* Orbit decomposition.

     We assume a choice of a representative for every orbit, and that
     the pieces are enumerated by orbit, so that the representative
     has index 0 in its orbit.

     Orbit i will be denoted by X_i, and its representative by r_i.
  */
  unsigned int num_orbits;
  unsigned int *orbit_size;
  unsigned int *orbit_offset;

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
};
typedef struct puzzle_t puzzle_t;

unsigned int puzzle_orbit_of(puzzle_t *puzzle, unsigned int x);
void puzzle_init(puzzle_t *puzzle,
                 unsigned int num_orbits, unsigned int *orbit_size,
                 group_t *group, uint8_t **orbit, uint8_t **stab);

#endif /* PUZZLE_H */
