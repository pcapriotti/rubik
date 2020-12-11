#ifndef DISPLAY_MEGAMINX_H
#define DISPLAY_MEGAMINX_H

#include <memory.h>
#include <stdint.h>
#include "linmath.h"

#include "lib/megaminx.h"

struct puzzle_scene_t;
typedef struct puzzle_scene_t puzzle_scene_t;

struct scene_t;
typedef struct scene_t scene_t;

puzzle_scene_t *megaminx_scene_new(scene_t *scene);

#endif /* DISPLAY_MEGAMINX_H */
