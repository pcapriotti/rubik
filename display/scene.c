#include "scene.h"
#include "piece.h"

#include <stdio.h>
#include <stdlib.h>

#define TB_RADIUS 0.7

void scene_init(scene_t *scene, unsigned int width, unsigned int height)
{
  scene->piece_cap = 16;
  scene->pieces = malloc(sizeof(piece_t) * scene->piece_cap);
  scene->num_pieces = 0;
  scene->tb_active = 0;

  quat_identity(scene->rot);
  memcpy(scene->lpos, (vec3) { 3, 4, 10 }, sizeof(vec3));
  mat4x4_translate(scene->view, 0.0, 0.0, -6.0);

  scene_resize(scene, width, height);
}

void scene_add_piece(scene_t *scene, piece_t *piece)
{
  if (scene->num_pieces >= scene->piece_cap) {
    scene->piece_cap <<= 1;
    scene->pieces = realloc(scene->pieces, sizeof(piece_t) * scene->piece_cap);
  }

  scene->pieces[scene->num_pieces++] = piece;
  scene_update_pieces(scene);
}

void tb_project(vec3 p, float x, float y)
{
  /* project onto a sphere */
  p[0] = x; p[1] = y; p[2] = 0.0;
  vec3_scale(p, p, 1 / TB_RADIUS);
  float r = vec3_len(p);
  if (r >= 1) {
    vec3_norm(p, p);
  }
  else {
    p[2] = sqrtf(1 - r * r);
  }
}

void scene_tb_start(scene_t *scene, float x, float y)
{
  scene->tb_active = 1;
  tb_project(scene->tb_down, x, y);
  memcpy(scene->tb_rot, scene->rot, sizeof(quat));
}

void scene_tb_update(scene_t *scene, float x, float y)
{
  vec3 p;
  tb_project(p, x, y);

  vec3 axis;
  quat q;
  vec3_mul_cross(axis, scene->tb_down, p);
  memcpy(q, axis, sizeof(vec3));
  q[3] = vec3_mul_inner(scene->tb_down, p);

  quat_mul(scene->rot, q, scene->tb_rot);
  scene_update_pieces(scene);
}

void scene_tb_end(scene_t *scene, float x, float y)
{
  scene_tb_update(scene, x, y);
  scene->tb_active = 0;
}

void scene_update_pieces(scene_t *scene)
{
  mat4x4 view_inv;
  mat4x4_invert(view_inv, scene->view);

  mat4x4 model;
  mat4x4_from_quat(model, scene->rot);

  for (unsigned int i = 0; i < scene->num_pieces; i++) {
    piece_update(scene->pieces[i], scene->proj, scene->view,
                 view_inv, model, scene->lpos);
  }
}

void scene_resize(scene_t *scene, unsigned int width, unsigned int height)
{
  mat4x4_perspective(scene->proj, sqrt(2) * 0.5, (float) width / (float) height,
                     0.1f, 100.0f);
  scene_update_pieces(scene);
}

void scene_render(scene_t *scene)
{
  for (unsigned int i = 0; i < scene->num_pieces; i++) {
    piece_render(scene->pieces[i]);
  }
}
