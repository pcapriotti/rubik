#include "scene.h"
#include "piece.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#define TB_RADIUS 0.7

void scene_init(scene_t *scene,
                unsigned int width, unsigned int height,
                double time0)
{
  scene->piece_cap = 16;
  scene->pieces = malloc(sizeof(piece_t) * scene->piece_cap);
  scene->num_pieces = 0;
  scene->tb_active = 0;
  scene->on_keypress = 0;
  scene->on_keypress_data = 0;
  scene->time0 = time0;

  memcpy(scene->rot, (quat) {-0.47, -0.36, -0.63, -0.50},
         sizeof(quat));

  memcpy(scene->data.lpos, (vec3) { 3, 4, 10 }, sizeof(vec3));
  mat4x4_translate(scene->data.view, 0.0, 0.0, -4.0);

  {
    glGenBuffers(1, &scene->data_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, scene->data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(scene->data), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_SCENE_DATA, scene->data_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

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
  /* printf("rot: (%.02f, %.02f, %.02f, %.02f)\n", */
  /*        scene->rot[0], */
  /*        scene->rot[1], */
  /*        scene->rot[2], */
  /*        scene->rot[3]); */
}

void scene_update_pieces(scene_t *scene)
{
  mat4x4 view_inv;
  mat4x4_invert(scene->data.view_inv, scene->data.view);

  mat4x4 model;
  mat4x4_from_quat(scene->data.model, scene->rot);

  glBindBuffer(GL_UNIFORM_BUFFER, scene->data_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(scene->data), &scene->data);
}

void scene_resize(scene_t *scene, unsigned int width, unsigned int height)
{
  mat4x4_perspective(scene->data.proj, sqrt(2) * 0.5, (float) width / (float) height,
                     0.1f, 100.0f);
  scene_update_pieces(scene);
}

void scene_render(scene_t *scene, double time)
{
  /* update time */
  scene->data.time = time - scene->time0;
  glBindBuffer(GL_UNIFORM_BUFFER, scene->data_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER,
                  offsetof(scene_data_t, time),
                  sizeof(float), &scene->data.time);

  for (unsigned int i = 0; i < scene->num_pieces; i++) {
    piece_render(scene->pieces[i], time);
  }
}
