#ifndef PIECE_H
#define PIECE_H

#include <memory.h>
#include "linmath.h"

struct poly_t;
typedef struct poly_t poly_t;

typedef struct
{
  unsigned int vao;
  unsigned int num_elements;
  unsigned int shader;

  quat q;
} piece_t;

void piece_render(piece_t *piece, unsigned int width, unsigned int height);
void piece_init(piece_t *piece, poly_t *poly, mat4x4 view, mat4x4 view_inv, vec3 lpos);
void piece_set_q(piece_t *piece, quat q);
void piece_set_proj(piece_t *piece, mat4x4 proj);

#endif /* PIECE_H */
