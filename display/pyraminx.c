#include "pyraminx.h"

#include "polyhedron.h"
#include "puzzle_scene.h"
#include "utils.h"

#include "lib/pyraminx.h"
#include "lib/group.h"
#include "lib/perm.h"
#include "lib/puzzle.h"

#include <stdlib.h>
#include <stdio.h>

void pyraminx_init_piece(poly_t *piece, poly_t *tetra, int type, int *facelets)
{
  abs_tetra(&piece->abs);
  piece->vertices = malloc(4 * sizeof(vec3));

  vec3 v0[4];

  for (unsigned int i = 1; i < 4; i++) {
    vec3_sub(v0[i], tetra->vertices[i], tetra->vertices[0]);
    vec3_scale(v0[i], v0[i], 1.0 / 3.0);
  }

  for (unsigned int i = 0; i < 4; i++) {
    facelets[i] = -1;
  }


  memcpy(piece->vertices, tetra->vertices, 4 * sizeof(vec3));

  switch (type) {
  case 0:
    memcpy(piece->vertices[0], tetra->vertices[0], sizeof(vec3));
    vec3_add(piece->vertices[1], piece->vertices[0], v0[1]);
    vec3_add(piece->vertices[2], piece->vertices[0], v0[2]);
    vec3_add(piece->vertices[3], piece->vertices[0], v0[3]);
    facelets[2] = 2; facelets[1] = 1; facelets[3] = 3;
    break;
  case 1:
    vec3_add(piece->vertices[0], tetra->vertices[0], v0[2]);
    vec3_add(piece->vertices[1], piece->vertices[0], v0[1]);
    vec3_add(piece->vertices[2], piece->vertices[0], v0[2]);
    vec3_add(piece->vertices[3], piece->vertices[0], v0[3]);
    facelets[1] = 1; facelets[3] = 3;
    break;
  case 2:
    vec3_add(piece->vertices[0], tetra->vertices[0], v0[1]);
    vec3_add(piece->vertices[1], piece->vertices[0], v0[2]);
    vec3_add(piece->vertices[2], tetra->vertices[0], v0[2]);
    vec3_add(piece->vertices[3], piece->vertices[0], v0[3]);
    facelets[3] = 3;
    break;
  }
}

static void pyraminx_model_init_piece(void *data, poly_t *poly,
                               unsigned int k, void *orbit,
                               int *facelets)
{
  poly_t *tetra = data;
  pyraminx_init_piece(poly, tetra, k, facelets);
}

static quat *pyraminx_rotations(puzzle_action_t *action, poly_t *tetra)
{
  quat *rots = malloc(action->group->num * sizeof(quat));

  quat_identity(rots[0]);
  for (unsigned int g = 1; g < action->group->num; g++) {
    unsigned int v0 = puzzle_action_local_act(action, 0, 0, g) % 4;
    unsigned int v1 = puzzle_action_local_act(action, 0, 1, g) % 4;

    if (v0 == 0) {
      rot_by_axis_and_vertices(rots[g], tetra->vertices[0],
                               tetra->vertices[1],
                               tetra->vertices[v1]);
    }
    else if (v1 == 1) {
      rot_by_axis_and_vertices(rots[g], tetra->vertices[1],
                               tetra->vertices[0],
                               tetra->vertices[v0]);
    }
    else {
      rot_by_vertices(rots[g],
                      tetra->vertices[0], tetra->vertices[1],
                      tetra->vertices[v0], tetra->vertices[v1]);
    }

    printf("v0: %u, v1: %u, rot: (%.02f, %.02f, %.02f, %.02f)\n",
           v0, v1, rots[g][0], rots[g][1], rots[g][2], rots[g][3]);
  }

  return rots;
}

static void pyraminx_model_cleanup(void *data, puzzle_model_t *model)
{
  free(model->init_piece_data);
  free(model->rots);
}

void pyraminx_model_init(puzzle_model_t *model, puzzle_action_t *action)
{
  poly_t *tetra = malloc(sizeof(poly_t));
  std_tetra(tetra);

  static vec4 colours[] = {
    { 1.0, 0.0, 0.0, 1.0 }, // red
    { 1.0, 1.0, 0.0, 1.0 }, // yellow
    { 0.0, 0.6, 0.0, 1.0 }, // green
    { 0.0, 0.0, 1.0, 1.0 }, // blue
  };

  model->init_piece = pyraminx_model_init_piece;
  model->init_piece_data = tetra;

  model->rots = pyraminx_rotations(action, tetra);
  model->colours = colours;

  model->cleanup = pyraminx_model_cleanup;
  model->cleanup_data = 0;
}

puzzle_scene_t *pyraminx_scene_new(scene_t *scene)
{
  puzzle_scene_t *s = malloc(sizeof(puzzle_scene_t));
  puzzle_action_t *action = malloc(sizeof(puzzle_action_t));

  pyraminx_action_init(action);
  printf("by stab[0]: ");
  debug_perm(action->by_stab[0], 12);
  printf("\ninv: ");
  debug_perm(action->inv_by_stab[0], 12);
  printf("\n");

  uint8_t *conf = pyraminx_new(action);

  puzzle_t *puzzle = malloc(sizeof(puzzle_t));
  pyraminx_puzzle_init(puzzle, action);

  puzzle_model_t *model = malloc(sizeof(puzzle_model_t));
  pyraminx_model_init(model, action);

  puzzle_scene_init(s, scene, conf, puzzle, model);

  /* static const unsigned char face_keys[] = "jfmvls,ckd;a"; */
  /* static const unsigned char rot_keys[] = "JFMVLS<CKD:A"; */
  /* for (unsigned int i = 0; i < 12; i++) { */
  /*   unsigned int f = i >> 1; */
  /*   int c = (i & 1) ? 1 : -1; */
  /*   puzzle_scene_set_move_binding(s, face_keys[i], f, c); */
  /*   puzzle_scene_set_rotation_binding(s, rot_keys[i], */
  /*                                     puzzle_action_stab(action, 2, f, c)); */
  /* } */

  return s;
}
