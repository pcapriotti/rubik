#ifndef DISPLAY_PYRAMINX_H
#define DISPLAY_PYRAMINX_H

struct puzzle_scene_t;
typedef struct puzzle_scene_t puzzle_scene_t;

struct scene_t;
typedef struct scene_t scene_t;

puzzle_scene_t *pyraminx_scene_new(scene_t *scene);

#endif /* DISPLAY_PYRAMINX_H */
