#ifndef PUZZLE_SCENE_H
#define PUZZLE_SCENE_H

#include <stdint.h>
#include <memory.h>
#include "linmath.h"

struct scene_t;
typedef struct scene_t scene_t;

struct puzzle_t;
typedef struct puzzle_t puzzle_t;

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

struct puzzle_model_t
{
  /* create piece model and return facelet data */
  void (*init_piece)(void *data, poly_t *poly, unsigned int k, void *orbit, int *facelets);
  void *init_piece_data;

  void (*cleanup)(void *data, struct puzzle_model_t *model);
  void *cleanup_data;

  quat *rots;
  vec4 *colours;
};
typedef struct puzzle_model_t puzzle_model_t;

struct puzzle_scene_t
{
  key_action_t *key_bindings;
  uint8_t *conf;
  piece_t *piece;
  puzzle_t *puzzle;
  puzzle_model_t *model;

  unsigned int count;
};

void puzzle_scene_init(puzzle_scene_t *s,
                       scene_t *scene,
                       uint8_t *conf,
                       puzzle_t *puzzle,
                       puzzle_model_t *model);
void puzzle_scene_cleanup(puzzle_scene_t *s);
void puzzle_scene_set_move_binding(puzzle_scene_t *s, unsigned char key,
                                   unsigned int f, int c);

#endif /* PUZZLE_SCENE_H */
