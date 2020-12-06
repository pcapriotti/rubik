#include "puzzle.h"
#include "group.h"

#include <assert.h>
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

unsigned int puzzle_orbit_of(puzzle_t *puzzle, unsigned int x)
{
  assert(x < puzzle->num_pieces);

  unsigned int lo = 0;
  unsigned int hi = puzzle->num_orbits;

  while (lo + 1 < hi) {
    unsigned int i = (hi + lo) / 2;
    if (puzzle->orbit_offset[i] > x)
      hi = i;
    else
      lo = i;
  }

  return lo;
}

unsigned int puzzle_act(void *data, unsigned int x, unsigned int g)
{
  puzzle_t *puzzle = data;
  unsigned int i = puzzle_orbit_of(puzzle, x);
  unsigned int stab = puzzle->group->num / puzzle->orbit_size[i];
  unsigned int j = x * stab;
  unsigned int g0 = puzzle->by_stab[i * puzzle->group->num + j];
  unsigned int g1 = group_mul(puzzle->group, g0, g);
  unsigned int j1 = puzzle->inv_by_stab[i * puzzle->group->num + g1];
  return j1 / stab;
}

void puzzle_init(puzzle_t *puzzle,
                 unsigned int num_orbits, unsigned int *orbit_size,
                 group_t *group, uint8_t *by_stab)
{
  puzzle->num_orbits = num_orbits;
  puzzle->orbit_size = malloc(puzzle->num_orbits * sizeof(unsigned int));
  memcpy(puzzle->orbit_size, orbit_size,
         puzzle->num_orbits * sizeof(unsigned int));
  puzzle->orbit_offset = malloc((puzzle->num_orbits + 1) * sizeof(unsigned int));
  puzzle->orbit_offset[0] = 0;
  for (unsigned int i = 1; i <= num_orbits; i++) {
    puzzle->orbit_offset[i] =
      puzzle->orbit_offset[i - 1] + puzzle->orbit_size[i - 1];
  }
  puzzle->num_pieces = puzzle->orbit_offset[num_orbits];
  puzzle->group = group;

  puzzle->by_stab = by_stab;
  puzzle->inv_by_stab = malloc(num_orbits * group->num);
  for (unsigned int i = 0; i < num_orbits; i++) {
    for (unsigned int j = 0; j < group->num; i++) {
      unsigned int g = by_stab[i * group->num + j];
      puzzle->inv_by_stab[i * group->num + g] = j;
    }
  }

  puzzle->action = malloc(sizeof(action_t));
  puzzle->action->data = puzzle;
  puzzle->action->cleanup = 0;
  puzzle->action->act = puzzle_act;
}

void puzzle_cleanup(puzzle_t *puzzle)
{
  action_cleanup(puzzle->action);
  group_cleanup(puzzle->group);

  free(puzzle->group);
  free(puzzle->action);
  free(puzzle->orbit_size);
  free(puzzle->orbit_offset);
  free(puzzle->by_stab);
  free(puzzle->inv_by_stab);
}
