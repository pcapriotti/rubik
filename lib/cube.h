#ifndef CUBE_H
#define CUBE_H

#include <stdint.h>
#include <stddef.h>

struct symmetries_t;
typedef struct symmetries_t symmetries_t;

struct orbit_t {
  unsigned int size;
  size_t offset;
  int dim;
};
typedef struct orbit_t orbit_t;

typedef struct
{
  unsigned int n;
  unsigned int num_corners;
  unsigned int num_edges;
  unsigned int num_centres;
  unsigned int num_pieces;

  unsigned int num_orbits;
  orbit_t *orbits;
} cube_shape_t;

void cube_shape_init(cube_shape_t *shape, unsigned int n);

struct cube_t
{
  cube_shape_t shape;
  uint8_t *pieces;
};
typedef struct cube_t cube_t;

void cube_init(symmetries_t *syms, cube_t *cube, unsigned int n);

uint8_t *cube_orbit(cube_t *cube, unsigned int k);

#endif /* CUBE_H */
