#ifndef SCENE_H
#define SCENE_H

#include <memory.h>
#include "linmath.h"

struct piece_t;
typedef struct piece_t piece_t;

typedef struct
{
  piece_t *piece;

  /* trackball information */
  vec3 tb_down; /* point of the trackball where dragging started */
  quat tb_rot;
  int tb_active;
} scene_t;

void scene_init(scene_t *scene, piece_t *piece);

void scene_tb_start(scene_t *scene, float x, float y);
void scene_tb_update(scene_t *scene, float x, float y);
void scene_tb_end(scene_t *scene, float x, float y);

#endif /* SCENE_H */
