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
  int x, y, z;
};
typedef struct orbit_t orbit_t;

typedef struct
{
  unsigned int n;
  unsigned int num_corners;
  unsigned int num_edges;
  unsigned int num_centres;
  unsigned int num_pieces;

  unsigned int num_corner_orbits;
  unsigned int num_edge_orbits;
  unsigned int num_centre_orbits;
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
cube_t *cube_generators(cube_t *cube, symmetries_t *syms);
void cube_act_(symmetries_t *syms, cube_t *cube, cube_t *move);
void cube_act(symmetries_t *syms, cube_t *cube1, cube_t *cube, cube_t *move);

#endif /* CUBE_H */
