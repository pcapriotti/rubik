#ifndef SCENE_H
#define SCENE_H

#include <memory.h>
#include "linmath.h"

typedef struct
{
  piece_t *piece;

  /* trackball information */
  vec3 tb_down; /* point of the trackball where dragging started */
} scene_t;

#endif /* SCENE_H */
