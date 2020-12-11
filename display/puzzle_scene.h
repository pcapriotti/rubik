#ifndef PUZZLE_SCENE_H
#define PUZZLE_SCENE_H

#include <stdint.h>
#include <memory.h>
#include "linmath.h"

struct puzzle_scene_t;
typedef struct puzzle_scene_t puzzle_scene_t;

struct poly_t;
typedef struct poly_t poly_t;

struct piece_t;
typedef struct piece_t piece_t;

struct key_action_t
{
  void (*run)(puzzle_scene_t *s, void *data);
  void *data;
};
typedef struct key_action_t key_action_t;

struct puzzle_scene_t
{
  key_action_t *key_bindings;
  uint8_t *conf;
  piece_t *piece;

  unsigned int count;
};

struct puzzle_model_t
{
  /* create piece model and return facelet data */
  void (*init)(void *data, poly_t *poly, void *orbit, int *facelets);
  void *init_data;

  quat *rots;
  vec4 *colours;
};
typedef struct puzzle_model_t puzzle_model_t;

#endif /* PUZZLE_SCENE_H */
