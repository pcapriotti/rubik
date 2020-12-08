#ifndef DISPLAY_MEGAMINX_H
#define DISPLAY_MEGAMINX_H

#include <memory.h>
#include <stdint.h>
#include "linmath.h"

#include "lib/megaminx.h"

struct poly_t;
typedef struct poly_t poly_t;

struct piece_t;
typedef struct piece_t piece_t;

struct scene_t;
typedef struct scene_t scene_t;

void megaminx_corner_poly(poly_t *mm, poly_t *dodec, float edge, int *facelets);
void megaminx_edge_poly(poly_t *mm, poly_t *dodec, float edge, int* facelets);
void megaminx_centre_poly(poly_t *mm, poly_t *dodec, float edge, int *facelets);

struct megaminx_scene_t;
typedef struct megaminx_scene_t megaminx_scene_t;

megaminx_scene_t *megaminx_scene_new(scene_t *scene);
void megaminx_scene_del(megaminx_scene_t *ms);

#endif /* DISPLAY_MEGAMINX_H */
