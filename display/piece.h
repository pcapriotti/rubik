#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>
#include <memory.h>
#include "linmath.h"

struct poly_t;
typedef struct poly_t poly_t;

struct turn_t;
typedef struct turn_t turn_t;

struct piece_t
{
  unsigned int vao;
  unsigned int num_elements;
  unsigned int instances;
  unsigned int rot_vbo;

  float duration;

  quat *rots;

  struct {
    double time0;
    quat *rot0;
    unsigned int num_pieces;
    unsigned int *pieces;
    unsigned int sym;
  } animation;
  unsigned int num_animations;

  /* per instance data */
  quat *rot_buf;
};
typedef struct piece_t piece_t;

void piece_render(piece_t *piece, double time);
void piece_init(piece_t *piece, poly_t *poly, int *facelets,
                quat *rots, uint8_t *conf, unsigned int instances);
void piece_cleanup(piece_t *piece);
void piece_set_conf(piece_t *piece, uint8_t *conf);
void piece_turn(piece_t *piece, unsigned int sym,
                unsigned int num_pieces, unsigned int *pieces);
void piece_cancel_animation(piece_t *piece);

#endif /* PIECE_H */
