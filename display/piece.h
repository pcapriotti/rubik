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
  unsigned int num_elements;
  unsigned int instances;
  unsigned int rot_vbo;

  float time;
  float duration;

  quat *rot_buf;

  /* per instance data */
  unsigned int *conf;
  unsigned int *conf1;
  float *start_time;
  quat *rots;
};
typedef struct piece_t piece_t;

void piece_render(piece_t *piece, float time);
void piece_init(piece_t *piece, poly_t *poly, int *facelets,
                quat *rots, uint8_t *conf, unsigned int instances);
void piece_cleanup(piece_t *piece);
void piece_set_conf(piece_t *piece, uint8_t *conf);
void piece_set_conf_instant(piece_t *piece, uint8_t *conf);

#endif /* PIECE_H */
