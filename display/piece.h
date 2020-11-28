#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>
#include <memory.h>
#include "linmath.h"

struct poly_t;
typedef struct poly_t poly_t;

struct piece_t
{
  unsigned int vao;
  unsigned int sym_vbo;
  unsigned int num_elements;
  unsigned int shader;
  unsigned int instances;

  mat4x4 model;
};
typedef struct piece_t piece_t;

void piece_render(piece_t *piece);
void piece_init(piece_t *piece, poly_t *poly, int *facelets,
                uint8_t *conf, unsigned int instances);
void piece_set_conf(piece_t *piece, uint8_t *conf);

#endif /* PIECE_H */
