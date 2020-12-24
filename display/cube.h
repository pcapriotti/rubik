#ifndef DISPLAY_CUBE_H
#define DISPLAY_CUBE_H

#include <memory.h>
#include "linmath.h"

struct scene_t;
typedef struct scene_t scene_t;

struct puzzle_scene_t;
typedef struct puzzle_scene_t puzzle_scene_t;

struct puzzle_action_t;
typedef struct puzzle_action_t puzzle_action_t;

quat *cube_puzzle_action_init(puzzle_action_t *action);

puzzle_scene_t *cube_scene_new(scene_t *scene, unsigned int n);
vec4 *cube_colours();

#endif /* DISPLAY_CUBE_H */
