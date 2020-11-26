#ifndef PIECE_H
#define PIECE_H

#include <memory.h>
#include "linmath.h"

struct poly_t;
typedef struct poly_t poly_t;

struct piece_t
{
  unsigned int vao;
  unsigned int num_elements;
  unsigned int shader;
  unsigned int instances;

  mat4x4 model;
};
typedef struct piece_t piece_t;

void piece_render(piece_t *piece);
void piece_init(piece_t *piece, poly_t *poly, int *facelets,
                unsigned int instances);
void piece_update(piece_t *piece, mat4x4 proj,
                  mat4x4 view, mat4x4 view_inv,
                  mat4x4 model, vec3 lpos);

#endif /* PIECE_H */
