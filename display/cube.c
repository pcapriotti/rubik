#include "cube.h"

#include <GL/glew.h>
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"
#include "piece.h"
#include "polyhedron.h"
#include "scene.h"
#include "puzzle_scene.h"

#include "lib/cube.h"
#include "lib/group.h"
#include "lib/perm.h"
#include "lib/puzzle.h"
#include "lib/utils.h"

void cube_piece_poly(poly_t *cube, unsigned int n,
                     unsigned int x, unsigned int y, unsigned int z,
                     int *facelets)
{
  std_cube(cube);
  mat4x4 m;

  float side = -2.0 * cube->vertices[0][0];
  float edge = 1 / (float) n;
  float gap = 0.03;
  float edge1 = (1 + gap) * edge;

  vec3 offset = {
    side * edge1 * (x - ((float) n - 1) / 2),
    side * edge1 * (y - ((float) n - 1) / 2),
    side * edge1 * (z - ((float) n - 1) / 2),
  };
  mat4x4_translate(m, offset[0], offset[1], offset[2]);
  mat4x4_scale_aniso(m, m, edge, edge, edge);

  for (unsigned int i = 0; i < cube->abs.num_vertices; i++) {
    vec4 v, w;
    memcpy(v, cube->vertices[i], sizeof(vec3));
    v[3] = 1.0;
    mat4x4_mul_vec4(w, m, v);
    memcpy(cube->vertices[i], w, sizeof(vec3));
  }

  unsigned int coords[3] = {x, y, z};
  for (unsigned int i = 0; i < 6; i++) {
    facelets[i] = (coords[i / 2] == (~i & 1) * (n - 1)) ? (int) i : -1;
  }

}

void quat_from_transp(quat q, unsigned int a, unsigned int b)
{
  memset(q, 0, sizeof(quat));
  if (a != b) {
    q[a] = 1;
    q[b] = -1;
  }
  else {
    q[3] = 1;
  }
}

void quat_from_perm(quat q, uint8_t *perm)
{
  /* transpose 0 and perm[0] */
  quat q0;
  quat_from_transp(q0, 0, perm[0]);

  uint8_t perm1[3];
  perm_id(perm1, 3);
  perm1[0] = perm[0];
  perm1[perm[0]] = 0;
  perm_mul(perm1, perm, 3);

  /* transpose 1 and 2 */
  quat q1;
  quat_from_transp(q1, 1, perm1[1]);

  quat_mul(q, q0, q1);
}

void quat_cube_sym(quat q, uint8_t s)
{
  for (unsigned int i = 0; i < 3; i++) {
    if (s & (1 << i)) {
      quat q1;
      memset(q1, 0, sizeof(quat));
      q1[i] = 1;

      quat tmp; memcpy(tmp, q, sizeof(quat));
      quat_mul(q, tmp, q1);
    }
  }
  quat_norm(q, q);
  /* printf("q: (%f, %f, %f, %f)\n", q[0], q[1], q[2], q[3]); */
}

void cube_mul_table(uint8_t *table, uint8_t *perm1, unsigned int s1)
{
  unsigned int index = 0;
  uint8_t perm2[3];
  for (unsigned int p1 = 0; p1 < 6; p1++) {
    perm_from_index(perm2, 3, p1, 3);
    unsigned int sign = perm_sign(perm2, 3);

    for (unsigned int j = 0; j < 4; j++) {
      unsigned int s2 = j | ((sign ^ (__builtin_popcount(j) & 1)) << 2);

      uint8_t perm[3];
      perm_composed(perm, perm1, perm2, 3);

      table[index++] = (perm_index(perm, 3, 3) << 2) |
        ((s2 ^ u16_conj(s1, perm2, 3)) & 0x3);
    }
  }
}

quat *cube_puzzle_action_init(puzzle_action_t *action)
{
  static const unsigned int num_syms = 24;

  /* The group G' of symmetries of a cube is the wreath product G' of
  Sigma_3 and O(1). G' acts linearly on the cube by permuting
  coordinates and changing signs. Explicitly, a pair (sigma, u) of a
  permutation and a choice of signs acts on the basis elements as
  follows:

    e_i (sigma, u) = u_i e_(sigma i).

  The determinant of (sigma, u) is sign(sigma) * sum(u). We are
  interested in the subgroup G of G' consisting of those elements that
  have determinant 1.

  They can be enumerated by enumerating the permutation sigma first,
  then choosing all possible signs for the first two axes, and setting
  the third sign to the only value that makes the determinant
  positive.

  To generate quaternions for all possible elements of G, we regard a
  quaternion as an element of Spin(3), and observe that Pin(3) is the
  direct product Spin(3) x O(1). Therefore, an element of Pin(3) can
  be represented by a pair of a quaternion and a sign. The strategy is
  then to write every element of G as a product of Pin(3) elements
  corresponding to reflections (i.e. tensors of rank 1 in the Clifford
  algebra).

  Since we know that this product is ultimately going to be
  a rotation, we can ignore the sign component, and simply use a
  quaternion to represent a reflection, namely the reflection
  corresponding to the composition of the usual rotation with scaling
  by -1 (i.e. reflection with respect to the origin).

  With these conventions, a reflection along a vector v corresponds to
  v itself regarded as a tensor, i.e. the purely imaginary quaternion
  associated to v.
 */

  group_t *group = malloc(sizeof(group_t));
  uint8_t *mul = malloc(num_syms * num_syms);
  quat *rots = malloc(num_syms * sizeof(quat));

  unsigned int index = 0;
  for (unsigned int p = 0; p < 6; p++) {
    uint8_t lehmer[3];
    uint8_t perm[3];

    lehmer_from_index(lehmer, 3, p, 3);
    perm_from_lehmer(perm, lehmer, 3);
    uint8_t sign = lehmer_sign(lehmer, 3);

    /* quaternion correponding to the element of Pin(3) determined by
    this permutation */
    quat q0;
    quat_from_perm(q0, perm);

    for (unsigned int j = 0; j < 4; j++) {
      unsigned int s = j | ((sign ^ (__builtin_popcount(j) & 1)) << 2);

      cube_mul_table(&mul[index * num_syms], perm, s);

      /* printf("[%u %u %u] %01x\n", perm[0], perm[1], perm[2], s); */
      /* printf("perm: (%f, %f, %f, %f)\n", q0[0], q0[1], q0[2], q0[3]); */

      memcpy(rots[index], q0, sizeof(quat));
      quat_cube_sym(rots[index], s);
      index++;
    }
  }

  group_from_table(group, num_syms, mul);

  unsigned int orbit_size[3] = { 8, 12, 6 };
  unsigned int stab_gen[3] = { 12, 5, 4 };

  uint8_t *stab[3];
  uint8_t *orbit[3];

  for (unsigned int k = 0; k < 3; k++) {
    orbit[k] = malloc(orbit_size[k]);
    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* vertices */
  for (unsigned int v = 0; v < 8; v++) {
    unsigned int p = __builtin_popcount(v) & 1;
    unsigned int s = (v & 1) | ((v >> p)& 2);
    orbit[0][v] = (p << 2) | s;
  }

  /* edges */
  for (unsigned int e = 0; e < 12; e++) {
    unsigned int a = e >> 2;
    unsigned int p = a & 1;
    unsigned int s = ((e & 1) << p) | ((e & 2) >> p);
    s = ((s & 1) << 1) | ((__builtin_popcount(s) & 1) ^ p);
    orbit[1][e] = (a << 3) | s;
  }

  /* faces */
  for (unsigned int f = 0; f < 6; f++) {
    uint8_t sign = (f >> 1) & 1;
    unsigned int s = f & 1;
    orbit[2][f] = ((f & ~1) << 2) | s;
  }

  puzzle_action_init(action, 3, orbit_size, group, orbit, stab);

  for (unsigned int k = 0; k < 3; k++) {
    free(stab[k]);
    free(orbit[k]);
  }

  return rots;
}


void cube_model_init_piece(void *data_, poly_t *poly,
                           unsigned int k, void *orbit_,
                           int *facelets)
{
  unsigned int *data = data_;
  unsigned int n = *data;
  orbit_t *orbit = orbit_;

  cube_piece_poly(poly, n, orbit->x, orbit->y, orbit->z, facelets);
}

void cube_model_cleanup(void *data, puzzle_model_t *model)
{
  free(model->init_piece_data);
  free(model->rots);
  free(model->colours);
}

void cube_model_init(puzzle_model_t *model, unsigned int n, quat *rots)
{
  model->init_piece = cube_model_init_piece;

  unsigned int *data = malloc(sizeof(unsigned int));
  *data = n;
  model->init_piece_data = data;

  model->rots = rots;
  vec4 colours[] = {
    { 1.0, 1.0, 1.0, 1.0 }, // white
    { 1.0, 1.0, 0.0, 1.0 }, // yellow
    { 0.0, 0.6, 0.0, 1.0 }, // green
    { 0.0, 0.0, 1.0, 1.0 }, // blue
    { 1.0, 0.0, 0.0, 1.0 }, // red
    { 1.0, 0.65, 0.0, 1.0 }, // orange
  };
  model->colours = malloc(sizeof(colours));
  memcpy(model->colours, colours, sizeof(colours));

  model->cleanup = cube_model_cleanup;
  model->cleanup_data = 0;
}

puzzle_scene_t *cube_scene_new(scene_t *scene, unsigned int n)
{
  puzzle_scene_t *s = malloc(sizeof(puzzle_scene_t));
  puzzle_action_t *action = malloc(sizeof(puzzle_action_t));
  quat *rots = cube_puzzle_action_init(action);
  cube_shape_t *shape = malloc(sizeof(cube_shape_t));
  cube_shape_init(shape, n);

  uint8_t *conf = cube_new(action, shape);

  puzzle_t *puzzle = malloc(sizeof(puzzle_t));
  cube_puzzle_init(puzzle, action, shape);
  puzzle_model_t *model = malloc(sizeof(puzzle_model_t));
  cube_model_init(model, n, rots);

  puzzle_scene_init(s, scene, conf, puzzle, model);

  static const unsigned char face_keys[] = "jfmvkd,cls;a";
  static const unsigned char rot_keys[] = "JFMVKD<CLS:A";
  for (unsigned int i = 0; i < 12; i++) {
    unsigned int f = i >> 1;
    int c = (i & 1) ? 1 : -1;
    puzzle_scene_set_move_binding(s, face_keys[i], f, c);
    puzzle_scene_set_rotation_binding(s, rot_keys[i],
                                      puzzle_action_stab(action, 2, f, c));
  }

  return s;
}
