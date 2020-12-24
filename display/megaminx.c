#include "megaminx.h"
#include "piece.h"
#include "polyhedron.h"
#include "scene.h"
#include "puzzle_scene.h"
#include "utils.h"

#include "lib/abs_poly.h"
#include "lib/group.h"
#include "lib/megaminx.h"
#include "lib/puzzle.h"

#include <GL/glew.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* extract a megaminx corner from vertex 0 of a dodecahedron */
/* edge is the ratio between the corner edge length and the full edge length */
static void megaminx_corner_poly(poly_t *mm, poly_t *dodec, float edge, int *facelets)
{
  abs_prism(&mm->abs, 4);

  vec3 p0, v1, v2, v3;
  memcpy(p0, dodec->vertices[0], sizeof(p0));

  vec3_sub(v1, dodec->vertices[1], p0); vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[4], p0); vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[5], p0); vec3_scale(v3, v3, edge);

  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));
  for (unsigned int i = 0; i < 8; i++) {
    memcpy(mm->vertices[i], p0, sizeof(vec3));
    if (i % 4 == 1 || i % 4 == 2) vec3_add(mm->vertices[i], mm->vertices[i], v1);
    if (i % 4 == 2 || i % 4 == 3) vec3_add(mm->vertices[i], mm->vertices[i], v2);
    if (i < 4) vec3_add(mm->vertices[i], mm->vertices[i], v3);
  }

  for (int i = 0; i < 6; i++) {
    facelets[i] = -1;
  }
  facelets[5] = 0;
  facelets[1] = 2;
  facelets[4] = 10;
}


static void megaminx_edge_poly(poly_t *mm, poly_t *dodec, float edge, int* facelets)
{
  abs_prism(&mm->abs, 4);
  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));

  vec3 v1, v2, v3;
  vec3_sub(v1, dodec->vertices[1], dodec->vertices[0]);
  vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[4], dodec->vertices[0]);
  vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[5], dodec->vertices[0]);
  vec3_scale(v3, v3, edge);

  vec3_add(mm->vertices[0], dodec->vertices[0], v1);
  vec3_add(mm->vertices[1], mm->vertices[0], v3);
  vec3_add(mm->vertices[2], mm->vertices[1], v2);
  vec3_add(mm->vertices[3], mm->vertices[0], v2);

  vec3_sub(v1, dodec->vertices[0], dodec->vertices[1]);
  vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[2], dodec->vertices[1]);
  vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[6], dodec->vertices[1]);
  vec3_scale(v3, v3, edge);

  vec3_add(mm->vertices[4], dodec->vertices[1], v1);
  vec3_add(mm->vertices[5], mm->vertices[4], v3);
  vec3_add(mm->vertices[6], mm->vertices[5], v2);
  vec3_add(mm->vertices[7], mm->vertices[4], v2);

  for (int i = 0; i < 6; i++) {
    facelets[i] = -1;
  }
  facelets[1] = 2;
  facelets[4] = 0;
}

static void megaminx_centre_poly(poly_t *mm, poly_t *dodec, float edge, int *facelets)
{
  abs_prism(&mm->abs, 5);
  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));

  for (unsigned int i = 0; i < 5; i++) {
    vec3 v1, v2, v3;
    vec3_sub(v1, dodec->vertices[(i + 1) % 5], dodec->vertices[i]);
    vec3_scale(v1, v1, edge);
    vec3_sub(v2, dodec->vertices[(i + 4) % 5], dodec->vertices[i]);
    vec3_scale(v2, v2, edge);
    vec3_sub(v3, dodec->vertices[i + 5], dodec->vertices[i]);
    vec3_scale(v3, v3, edge);

    vec3_add(mm->vertices[i + 5], dodec->vertices[i], v1);
    vec3_add(mm->vertices[i + 5], mm->vertices[i + 5], v2);
    vec3_add(mm->vertices[i], mm->vertices[i + 5], v3);
  }

  for (int i = 0; i < 10; i++) {
    facelets[i] = -1;
  }
  facelets[6] = 0;
}

quat *megaminx_rotations(poly_t *dodec)
{
  quat *rots = malloc(60 * sizeof(quat));

  /* symmetry i * 5 + j maps face 0 to i, and face 2 to the
  j-th face (in order from the lowest-numbered face) adjacent to i */

  quat rho;
  quat_rotate(rho, 2 * M_PI / 5, (vec3) { 1, 0, 0 });

  /* swap faces 0 and 2 */
  quat tau;
  rot_by_vertices(tau, dodec->vertices[0], dodec->vertices[1],
                  dodec->vertices[1], dodec->vertices[0]);

  quat_identity(rots[0]);
  /* map face 0 to 1 and 2 to 3 */
  rot_by_vertices(rots[5], dodec->vertices[0], dodec->vertices[1],
                  dodec->vertices[18], dodec->vertices[19]);

  for (unsigned int k = 0; k < 2; k++) {
    quat_mul(rots[(2 + k) * 5], rots[k * 5], tau);
    for (unsigned int j = 2; j <= 5; j++) {
      quat_mul(rots[(2 * j + k) * 5], rho, rots[(2 * (j - 1) + k) * 5]);
    }
  }

  for (unsigned int j = 0; j < 12; j++) {
    for (unsigned int i = 1; i < 5; i++) {
      quat_mul(rots[j * 5 + i], rots[j * 5 + i - 1], rho);
    }
  }

  return rots;
}

static void dodecahedron_group_init(group_t *group, abs_poly_t *dodec, poly_data_t *data)
{
  static const unsigned int num = 60;
  uint8_t *mul = malloc(num * num);

  for (unsigned int s = 0; s < num; s++) {
    uint8_t *table = &mul[num * s];

    unsigned int f0 = s / 5;
    unsigned int vi = s % 5;

    for (unsigned int j = 0; j < 5; j++) {
      table[j] = f0 * 5 + (vi + j) % 5;
      table[5 + j] = (f0 ^ 1) * 5 + (5 - vi + j) % 5;
    }

    int vi0 = data->adj[f0 * dodec->num_vertices + data->first_vertex[f0]];
    assert(vi0 != -1);
    for (unsigned int i = 0; i < 5; i++) {
      unsigned int v = dodec->faces[f0].vertices[(vi0 + vi + i) % 5];
      unsigned int w = dodec->faces[f0].vertices[(vi0 + vi + i + 1) % 5];
      unsigned int f1 = data->edges[w * dodec->num_vertices + v];

      int wi0 = data->adj[f1 * dodec->num_vertices + data->first_vertex[f1]];
      assert(wi0 != -1);
      int wi = data->adj[f1 * dodec->num_vertices + w];
      assert(wi != -1);

      for (unsigned int j = 0; j < 5; j++) {
        table[10 * i + 10 + j] = f1 * 5 + (wi - wi0 + j + 5) % 5;
        table[10 * i + 15 + j] = (f1 ^ 1) * 5 + (wi0 - wi + j + 5) % 5;
      }
    }
  }

  group_from_table(group, num, mul);
}

void megaminx_puzzle_action_init(puzzle_action_t *puzzle, abs_poly_t *dodec, poly_data_t *data)
{
  const unsigned int num_syms = 60;
  unsigned int orbit_size[] = { 20, 30, 12 };
  unsigned int stab_gen[] = { 50, 10, 1};

  group_t *group = malloc(sizeof(group_t));
  dodecahedron_group_init(group, dodec, data);

  uint8_t *orbit[3];
  uint8_t *stab[3];

  for (unsigned int k = 0; k < 3; k++) {
    orbit[k] = malloc(orbit_size[k]);
    memset(orbit[k], num_syms, orbit_size[k]);

    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* faces */
  for (unsigned int i = 0; i < orbit_size[2]; i++) {
    orbit[2][i] = i * 5;
  }

  /* vertices */
  {
    for (unsigned int j = 0; j < orbit_size[2]; j++) {
      unsigned int n = dodec->faces[j].num_vertices;
      for (unsigned int i = 0; i < n; i++) {
        unsigned int v = dodec->faces[j].vertices[i];
        unsigned int f = j;
        if (orbit[0][v] != num_syms) continue;
        int i0 = data->adj[f * dodec->num_vertices + data->first_vertex[f]];
        assert(i0 != -1);
        unsigned int s = f * 5 + (i - i0 + 5) % 5;
        orbit[0][v] = s;
      }
    }
  }

  /* edges */
  {
    unsigned int index = 0;
    for (unsigned int f = 0; f < dodec->num_faces; f++) {
      unsigned int n = dodec->faces[f].num_vertices;
      int vi0 = data->adj[f * dodec->num_vertices + data->first_vertex[f]];
      for (unsigned int i = 0; i < n; i++) {
        unsigned int f1 = abs_poly_get_adj_face(dodec, f, i, data->edges);
        if (f1 < f) continue;
        orbit[1][data->edges_by_face[f][i]] = f * 5 + (i - vi0 + 5) % 5;
      }
    }
  }

  puzzle_action_init(puzzle, 3, orbit_size, group, orbit, stab);

  for (unsigned int k = 0; k < 3; k++) {
    free(stab[k]);
    free(orbit[k]);
  }
}

void megaminx_model_init_piece(void *data, poly_t *poly,
                               unsigned int k, void *orbit,
                               int *facelets)
{
  static const float edge_size = 0.4;
  poly_t *dodec = data;
  switch (k) {
  case 0:
    megaminx_corner_poly(poly, dodec, edge_size, facelets);
    break;
  case 1:
    megaminx_edge_poly(poly, dodec, edge_size, facelets);
    break;
  case 2:
    megaminx_centre_poly(poly, dodec, edge_size, facelets);
    break;
  }
}

void megaminx_model_cleanup(void *data, puzzle_model_t *model)
{
  free(model->rots);
  free(model->colours);

  poly_t *dodec = model->init_piece_data;
  poly_cleanup(dodec);
  free(dodec);
}

static void megaminx_model_init(puzzle_model_t *model,
                                poly_t* dodec, decomp_t *decomp)
{
  model->rots = megaminx_rotations(dodec);
  vec4 colours[] = {
    { 1.0, 1.0, 1.0, 1.0 }, // white
    { 0.4, 0.4, 0.4, 1.0 }, // grey
    { 1.0, 1.0, 0.0, 1.0 }, // yellow
    { 0.91, 0.85, 0.68, 1.0 }, // pale yellow
    { 0.5, 0.0, 0.5, 1.0 }, // purple
    { 1.0, 0.75, 0.8, 1.0 }, // pink
    { 0.0, 0.6, 0.0, 1.0 }, // green
    { 0.2, 0.8, 0.2, 1.0 }, // lime
    { 1.0, 0.0, 0.0, 1.0 }, // red
    { 1.0, 0.65, 0.0, 1.0 }, // orange
    { 0.0, 0.0, 1.0, 1.0 }, // blue
    { 0.0, 0.5, 0.5, 1.0 }, // teal
  };
  model->colours = malloc(sizeof(colours));
  memcpy(model->colours, colours, sizeof(colours));

  model->init_piece = megaminx_model_init_piece;
  model->init_piece_data = dodec;
  model->cleanup = megaminx_model_cleanup;
  model->cleanup_data = 0;
  model->decomp = decomp;
}

puzzle_scene_t *megaminx_scene_new(scene_t *scene)
{
  puzzle_scene_t *s = malloc(sizeof(puzzle_scene_t));

  puzzle_action_t *action = malloc(sizeof(puzzle_action_t));

  poly_t *dodec = malloc(sizeof(poly_t));
  std_dodec(dodec);

  poly_data_t poly_data;
  poly_data_init(&poly_data, &dodec->abs);
  megaminx_puzzle_action_init(action, &dodec->abs, &poly_data);
  poly_data_cleanup(&poly_data);

  uint8_t *conf = megaminx_new(action);

  puzzle_t *puzzle = malloc(sizeof(puzzle_t));
  megaminx_puzzle_init(puzzle, action);
  puzzle_model_t *model = malloc(sizeof(puzzle_model_t));
  megaminx_model_init(model, dodec, &action->decomp);

  puzzle_scene_init(s, scene, conf, puzzle, model);
  static const unsigned char face_keys[] = "jfnbkdurmv.x,c/z;aielsow";
  static const unsigned char rot_keys[] = "JFNBKDURMV>X<C?Z:AIELSOW";
  for (unsigned int i = 0; i < 24; i++) {
    puzzle_scene_set_move_binding(s, face_keys[i], i >> 1, (i & 1) ? 1 : -1, -1);
    puzzle_scene_set_rotation_binding(s, rot_keys[i],
                                      puzzle_action_stab(action, 2,
                                                         i >> 1, (i & 1) ? 1 : -1));
  }

  /* rotate puzzle so that the front face is actually on the front */
  {
    unsigned int sym = puzzle_action_stab(action, 2, 0, 1);
    quat q;
    quat_mul(q, scene->rot, model->rots[sym]);
    memcpy(scene->rot, q, sizeof(quat));
  }

  return s;
}
