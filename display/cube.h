#ifndef DISPLAY_CUBE_H
#define DISPLAY_CUBE_H

struct poly_t;
typedef struct poly_t poly_t;

struct scene_t;
typedef struct scene_t scene_t;

static const unsigned int cube_num_syms = 24;

void cube_scene_new(scene_t *scene, unsigned int n);

#endif /* DISPLAY_CUBE_H */
