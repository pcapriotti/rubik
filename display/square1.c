#include "square1.h"

#include <stdlib.h>

#include "polyhedron.h"
#include "puzzle_scene.h"
#include "scene.h"
#include "cube.h"

#include "lib/puzzle.h"
#include "lib/square1.h"
#include "lib/abs_poly.h"

enum {
  CORNER,
  EDGE,
  MIDDLE
};

static void square1_poly(poly_t *poly, float side, unsigned int type, int *facelets)
{
  const unsigned int num = type == EDGE ? 3 : 4;
  abs_prism(&poly->abs, num);
  poly->vertices = malloc(num * 2 * sizeof(vec3));

  float x = side * 0.5;
  float y = x * tanf(M_PI / 12);

  for (unsigned int i = 0; i < 6; i++) {
    facelets[i] = -1;
  }

  for (unsigned int i = 0; i < 2; i++) {
    float z;
    switch (type) {
    case CORNER:
      z = i ? 0.5 : 0.16666;
      memcpy(poly->vertices[4 * i + 0], (vec3) { z, x, y }, sizeof(vec3));
      memcpy(poly->vertices[4 * i + 1], (vec3) { z, x, x }, sizeof(vec3));
      memcpy(poly->vertices[4 * i + 2], (vec3) { z, y, x }, sizeof(vec3));
      memcpy(poly->vertices[4 * i + 3], (vec3) { z, 0, 0 }, sizeof(vec3));
      facelets[5] = 0; facelets[1] = 1; facelets[2] = 2;
      break;
    case EDGE:
      z = i ? 0.5 : 0.16666;
      memcpy(poly->vertices[3 * i + 0], (vec3) { z, y, x }, sizeof(vec3));
      memcpy(poly->vertices[3 * i + 1], (vec3) { z, -y, x }, sizeof(vec3));
      memcpy(poly->vertices[3 * i + 2], (vec3) { z, 0, 0 }, sizeof(vec3));
      facelets[4] = 0; facelets[1] = 2;
      break;
    case MIDDLE:
      z = i ? 0.16666 : -0.16666;
      memcpy(poly->vertices[4 * i + 0], (vec3) { z, y, -x }, sizeof(vec3));
      memcpy(poly->vertices[4 * i + 1], (vec3) { z, x, -x }, sizeof(vec3));
      memcpy(poly->vertices[4 * i + 2], (vec3) { z, x, x }, sizeof(vec3));
      memcpy(poly->vertices[4 * i + 3], (vec3) { z, -y, x }, sizeof(vec3));
      facelets[1] = 4; facelets[2] = 1; facelets[3] = 2;
      break;
    }
  }
}

static void square1_model_cleanup(void *data, puzzle_model_t *model)
{
  free(model->rots);
}

static void square1_init_piece(void *data, poly_t *poly,
                               unsigned int k, void *orbit, int *facelets)
{
  square1_poly(poly, 1.0, k, facelets);
}

quat *square1_rotations()
{
  quat *rots = malloc(24 * sizeof(quat));

  quat r;
  quat_rotate(r, M_PI / 6, (vec3) {1, 0, 0});

  quat_identity(rots[0]);
  quat_rotate(rots[1], M_PI, (vec3) { 0, 1 / sqrtf(2), 1 / sqrtf(2) });

  for (unsigned int i = 1; i < 12; i++) {
    quat_mul(rots[i * 2], r, rots[(i - 1) * 2]);
    quat_mul(rots[i * 2 + 1], rots[i * 2], rots[1]);
  }

  return rots;
}

puzzle_scene_t *square1_scene_new(scene_t *scene)
{
  puzzle_scene_t *s = malloc(sizeof(puzzle_scene_t));

  puzzle_action_t *action = malloc(sizeof(puzzle_action_t));
  square1_action_init(action);

  puzzle_t *puzzle = malloc(sizeof(puzzle_t));
  square1_puzzle_init(puzzle, action);

  uint8_t *conf = square1_new(puzzle->decomp);

  puzzle_model_t *model = malloc(sizeof(puzzle_model_t));
  model->init_piece = square1_init_piece;
  model->init_piece_data = 0;
  model->cleanup = square1_model_cleanup;
  model->cleanup_data = 0;
  model->rots = square1_rotations();
  model->colours = cube_colours();
  model->num_colours = 6;
  model->decomp = puzzle->decomp;

  puzzle_scene_init(s, scene, conf, puzzle, model);

  static const unsigned char face_keys[] = "jfmvlskd";
  static const unsigned char rot_keys[] = "JFMVLSKD";
  static const unsigned int rot_syms[] = { 2, 22, 22, 2, 2, 22, 3, 3 };
  for (unsigned int i = 0; i < 8; i++) {
    unsigned int f = i >> 1;
    int c = (i & 1) ? 1 : -1;
    puzzle_scene_set_move_binding(s, face_keys[i], f, c, -1);
    puzzle_scene_set_rotation_binding(s, rot_keys[i], rot_syms[i]);
  }


  return s;
}
