#include "puzzle.h"
#include "group.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

unsigned int symmetries_num_cells(symmetries_t *syms,
                                  unsigned int dim)
{
  switch (dim) {
  case 0:
    return syms->num_vertices;
  case 1:
    return syms->num_edges;
  case 2:
    return syms->num_faces;
  }
  return 0;
}

unsigned int symmetries_by_cell(symmetries_t *syms,
                                unsigned int dim,
                                unsigned int i)
{
  switch (dim) {
  case 0:
    return syms->by_vertex[i];
  case 1:
    return syms->by_edge[i];
  case 2:
    return syms->by_face[i];
  }

  return 0;
}

/* act on a geometric cell */
unsigned int symmetries_cell_act(symmetries_t* syms,
                                 unsigned int dim,
                                 unsigned int i,
                                 unsigned int s)
{
  switch (dim) {
  case 0:
    return syms->vertex_action[s * syms->num_vertices + i];
  case 1:
    return syms->edge_action[s * syms->num_edges + i];
  case 2:
    return syms->face_action[s * syms->num_faces + i];
  }

  return 0;
}

/* act on a piece
   dim: dimension
   i: index of the piece
   stab: index of the stabiliser
   s: symmetry */
unsigned int symmetries_act(symmetries_t *syms,
                            unsigned int dim,
                            unsigned int i,
                            unsigned int stab,
                            unsigned int s)
{
  return 0;
}

unsigned int decomp_orbit_of(decomp_t *decomp, unsigned int x)
{
  assert(x < decomp->num_pieces);

  unsigned int lo = 0;
  unsigned int hi = decomp->num_orbits;

  while (lo + 1 < hi) {
    unsigned int i = (hi + lo) / 2;
    if (decomp->orbit_offset[i] > x)
      hi = i;
    else
      lo = i;
  }

  return lo;
}

/* global piece index from orbit and local index */
unsigned int decomp_global(decomp_t *decomp, unsigned int i, unsigned int j)
{
  return decomp->orbit_offset[i] + j;
}

/* local piece index */
unsigned int decomp_local(decomp_t *decomp, unsigned int j)
{
  unsigned int i = decomp_orbit_of(decomp, j);
  return j - decomp->orbit_offset[i];
}

/* representative (i.e. first element) of an orbit */
unsigned int decomp_repr(decomp_t *decomp, unsigned int i)
{
  return decomp->orbit_offset[decomp_orbit_of(decomp, i)];
}

unsigned int puzzle_act(puzzle_t *puzzle, unsigned int x, unsigned int g)
{
  unsigned int i = decomp_orbit_of(&puzzle->decomp, x);
  x -= puzzle->decomp.orbit_offset[i];
  unsigned int g0 = puzzle->by_stab[i][x];
  unsigned int g1 = group_mul(puzzle->group, g0, g);
  unsigned int j = puzzle->inv_by_stab[i][g1];
  return puzzle->decomp.orbit_offset[i] + j % puzzle->decomp.orbit_size[i];
}

void puzzle_init(puzzle_t *puzzle,
                 unsigned int num_orbits, unsigned int *orbit_size,
                 group_t *group, uint8_t **orbit, uint8_t **stab)
{
  puzzle->decomp.num_orbits = num_orbits;
  puzzle->decomp.orbit_size =
    malloc(puzzle->decomp.num_orbits * sizeof(unsigned int));
  memcpy(puzzle->decomp.orbit_size, orbit_size,
         puzzle->decomp.num_orbits * sizeof(unsigned int));
  puzzle->decomp.orbit_offset =
    malloc((puzzle->decomp.num_orbits + 1) * sizeof(unsigned int));
  puzzle->decomp.orbit_offset[0] = 0;
  for (unsigned int i = 1; i <= num_orbits; i++) {
    puzzle->decomp.orbit_offset[i] =
      puzzle->decomp.orbit_offset[i - 1] + puzzle->decomp.orbit_size[i - 1];
  }
  puzzle->decomp.num_pieces = puzzle->decomp.orbit_offset[num_orbits];
  puzzle->group = group;

  puzzle->by_stab = malloc(num_orbits * sizeof(uint8_t *));
  puzzle->inv_by_stab = malloc(num_orbits * sizeof(uint8_t *));
  for (unsigned int i = 0; i < num_orbits; i++) {
    puzzle->by_stab[i] = malloc(group->num);
    puzzle->inv_by_stab[i] = malloc(group->num);
    for (unsigned int j = 0; j < group->num; j++) {
      unsigned int g =
        group_mul(puzzle->group,
                  stab[i][j / puzzle->decomp.orbit_size[i]],
                  orbit[i][j % puzzle->decomp.orbit_size[i]]);
      puzzle->by_stab[i][j] = g;
      puzzle->inv_by_stab[i][g] = j;
    }
  }
}

void puzzle_cleanup(puzzle_t *puzzle)
{
  group_cleanup(puzzle->group);

  free(puzzle->group);
  free(puzzle->decomp.orbit_size);
  free(puzzle->decomp.orbit_offset);
  free(puzzle->by_stab);
  free(puzzle->inv_by_stab);
}

void turn_del(turn_t *turn)
{
  free(turn->pieces);
  free(turn);
}
