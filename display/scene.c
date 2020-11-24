#include "scene.h"
#include "piece.h"

#include <stdio.h>

#define TB_RADIUS 0.7

void scene_init(scene_t *scene, piece_t *piece)
{
  scene->piece = piece;
  scene->tb_active = 0;
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
  memcpy(scene->tb_rot, scene->piece->q, sizeof(quat));
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

  quat q1;
  quat_mul(q1, q, scene->tb_rot);
  piece_set_q(scene->piece, q1);
}

void scene_tb_end(scene_t *scene, float x, float y)
{
  scene_tb_update(scene, x, y);
  scene->tb_active = 0;
}
