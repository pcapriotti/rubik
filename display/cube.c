#include "cube.h"

#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"
#include "polyhedron.h"

#include "lib/perm.h"
#include "lib/puzzle.h"

void cube_piece_poly(poly_t *cube, float edge, int *facelets)
{
  std_cube(cube);
  mat4x4 m;
  mat4x4_identity(m);
  mat4x4_scale_aniso(m, m, edge, edge, edge);
  memcpy(m[3], cube->vertices[0], sizeof(vec3));
  vec3_scale(m[3], m[3], 1 - edge);

  for (unsigned int i = 0; i < cube->abs.num_vertices; i++) {
    vec4 v, w;
    memcpy(v, cube->vertices[i], sizeof(vec3));
    v[3] = 1.0;
    mat4x4_mul_vec4(w, m, v);
    memcpy(cube->vertices[i], w, sizeof(vec3));
  }
}

int gcd(int m, int n)
{
  while (m) {
    int x = m;
    m = n % m;
    n = x;
  }
  return n;
}

void quat_cube_sym(quat q, uint8_t *perm, uint8_t s)
{
  uint8_t visited = 0;

  /* find positive cycle */
  int found = 0;

  vec3 axis;
  int sign;
  unsigned int length;
  unsigned int order = 1;

  for (unsigned int i = 0; i < 3; i++) {
    length = 0;
    sign = 1;
    if (!found) memset(axis, 0, sizeof(vec3));

    while (1) {
      uint8_t mask = 1 << i;
      if (visited & mask) {
        if (sign == 1 && length > 0) found = 1;

        unsigned int order0 = length;
        if (sign == -1) order0 *= 2;
        if (length > 0)
          order *= order0 / gcd(order0, order);

        break;
      }
      visited |= mask;
      sign *= (s & mask) ? -1 : 1;

      if (!found) {
        vec3 v = { 0, 0, 0 };
        v[i] = 1;

        vec3_scale(v, v, sign);
        vec3_add(axis, axis, v);
      }

      i = perm[i];
      length++;
    }
  }
  assert(found);

  vec3_norm(axis, axis);
  quat_rotate(q, 2 * M_PI / order, axis);
}

void cube_syms_init(symmetries_t *syms, poly_t *cube)
{
  quat *rots = malloc(cube_num_syms * sizeof(quat));

  unsigned int index = 0;
  for (unsigned int p = 0; p < 6; p++) {
    uint8_t perm[3];
    perm_from_index(perm, 3, p, 3);
    uint8_t sign = perm_sign(perm, 3);

    for (unsigned int s = 0; s < 8; s++) {
      if (__builtin_popcount(s) % 2 != sign) continue;

      /* quaternion */
      quat_cube_sym(rots[index], perm, s);

      /* face action */
    }
  }
}

void cube_scene_new(scene_t *scene, unsigned int n)
{
  int facelets[6];
  poly_t cube;
  cube_piece_poly(&cube, 1.0 / (float) n, facelets);
  poly_debug(&cube);

  symmetries_t syms;
  cube_syms_init(&syms, &cube);
}
