#include "bump_cube.h"

#include <stdlib.h>

#include "lib/puzzle.h"
#include "lib/cube.h"

#include "cube.h"
#include "polyhedron.h"
#include "puzzle_scene.h"

void bump_cube_model_cleanup(void *data, puzzle_model_t *model)
{
  free(model->decomp);
  free(model->rots);
}

int *bump_cube_model_init_piece(void *data_, poly_t *poly,
                                unsigned int k, void *orbit_)
{
  static float sizex[] = { 0.4, 0.35, 0.25 };
  static float sizey[] = { 0.5, 0.3, 0.2 };
  static float sizez[] = { 0.6, 0.22, 0.18 };

  std_cube(poly);

  orbit_t *orbit = orbit_;


  float side = fabs(2.0 * poly->vertices[0][0]);
  float edge = 1 / 3.0;

  vec3 offset = {
    side * edge * (orbit->x - 1),
    side * edge * (orbit->y - 1),
    side * edge * (orbit->z - 1)
  };
  mat4x4 m;
  mat4x4_translate(m, offset[0], offset[1], offset[2]);
  mat4x4_scale_aniso(m, m,
                     edge * sizex[orbit->x],
                     edge * sizey[orbit->y],
                     edge * sizez[orbit->z]);

  poly_trans(poly, m);

  unsigned int coords[3] = {orbit->x, orbit->y, orbit->z};
  int *facelets = malloc(poly->abs.num_faces * sizeof(int));
  for (unsigned int i = 0; i < 6; i++) {
    facelets[i] = (coords[i / 2] == (~i & 1) * 2) ? 0 : -1;
  }

  return facelets;
}

static void *bump_cube_orbit(void *data, unsigned int i)
{
  cube_shape_t *shape = data;

  unsigned int k = decomp_orbit_of(&shape->decomp, i);
  return &shape->orbits[k];
}

unsigned int bump_cube_facelet(void *data, unsigned int k,
                               unsigned int x, unsigned int i)
{
  return 0;
}

void bump_cube_model_init(puzzle_model_t *model, quat *rots,
                          cube_shape_t *shape)
{
  model->init_piece = bump_cube_model_init_piece;
  model->init_piece_data = 0;

  model->orbit = bump_cube_orbit;
  model->orbit_data = shape;

  model->facelet = bump_cube_facelet;
  model->facelet_data = 0;

  static vec4 colours[] = {
    /* all white */
    { 1.0, 1.0, 1.0, 1.0 },
  };

  model->rots = rots;
  model->colours = colours;
  model->num_colours = 1;

  decomp_t *tdecomp = malloc(sizeof(decomp_t));
  decomp_init_trivial(tdecomp, shape->decomp.num_pieces);
  model->decomp = tdecomp;

  model->cleanup = bump_cube_model_cleanup;
  model->cleanup_data = 0;
}

puzzle_scene_t *bump_cube_scene_new(scene_t *scene)
{
  puzzle_scene_t *s = malloc(sizeof(puzzle_scene_t));
  puzzle_action_t *action = malloc(sizeof(puzzle_action_t));
  quat *rots = cube_puzzle_action_init(action);
  cube_shape_t *shape = malloc(sizeof(cube_shape_t));
  cube_shape_init(shape, 3);

  uint8_t *conf = cube_new(action, shape);

  puzzle_t *puzzle = malloc(sizeof(puzzle_t));
  cube_puzzle_init(puzzle, action, shape);
  puzzle_model_t *model = malloc(sizeof(puzzle_model_t));
  bump_cube_model_init(model, rots, shape);

  puzzle_scene_init(s, scene, conf, puzzle, model);

  static const unsigned char face_keys[] = "jfmvls,ckd;a";
  static const unsigned char rot_keys[] = "JFMVLS<CKD:A";
  for (unsigned int i = 0; i < 12; i++) {
    unsigned int f = i >> 1;
    int c = (i & 1) ? 1 : -1;
    puzzle_scene_set_move_binding(s, face_keys[i], f, c, -1);
    puzzle_scene_set_rotation_binding(s, rot_keys[i],
                                      puzzle_action_stab(action, 2, f, c));
  }

  return s;
}
