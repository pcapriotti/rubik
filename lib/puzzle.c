#include "puzzle.h"

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
