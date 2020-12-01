#include "puzzle.h"

unsigned int symmetries_by_cell(symmetries_t *syms, unsigned int dim, unsigned int i)
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
