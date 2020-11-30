#ifndef CUBE_H
#define CUBE_H

#include <stdint.h>

struct symmetries_t;
typedef struct symmetries_t symmetries_t;

typedef struct
{
  unsigned int n;
  unsigned int num_corners;
  unsigned int num_edges;
  unsigned int num_centres;
} cube_shape_t;

void cube_shape_init(cube_shape_t *shape, unsigned int n);

struct cube_t
{
  cube_shape_t shape;
  uint8_t *pieces;
};
typedef struct cube_t cube_t;

void cube_init(symmetries_t *syms, cube_t *cube, unsigned int n);
uint8_t *cube_centre(cube_t *cube, unsigned int f);
uint8_t *cube_edge(cube_t *cube, unsigned int e);
uint8_t *cube_corner(cube_t *cube, unsigned int c);

#endif /* CUBE_H */
