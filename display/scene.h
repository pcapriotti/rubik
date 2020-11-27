#ifndef SCENE_H
#define SCENE_H

#include <memory.h>
#include "linmath.h"

struct piece_t;
typedef struct piece_t piece_t;

typedef struct
{
  mat4x4 view;
  mat4x4 view_inv;
  mat4x4 proj;
  mat4x4 model;
  vec3 lpos;
} __attribute__((packed, aligned(4))) scene_data_t;

enum {
  BINDING_SCENE_DATA,
  BINDING_SYMS,
  BINDING_FACE_ACTION,
};

struct scene_t
{
  piece_t **pieces;
  unsigned int num_pieces;
  unsigned int piece_cap;

  scene_data_t data;
  unsigned int data_ubo;
  quat rot;

  /* trackball information */
  vec3 tb_down; /* point of the trackball where dragging started */
  quat tb_rot;
  int tb_active;
};
typedef struct scene_t scene_t;

void scene_init(scene_t *scene, unsigned int width, unsigned int height);
void scene_add_piece(scene_t *scene, piece_t *piece);
void scene_resize(scene_t *scene, unsigned int width, unsigned int height);
void scene_render(scene_t *scene);

void scene_tb_start(scene_t *scene, float x, float y);
void scene_tb_update(scene_t *scene, float x, float y);
void scene_tb_end(scene_t *scene, float x, float y);

void scene_update_pieces(scene_t *scene);

#endif /* SCENE_H */
