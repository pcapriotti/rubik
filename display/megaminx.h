#ifndef MEGAMINX_H
#define MEGAMINX_H

#include <memory.h>
#include <stdint.h>
#include "linmath.h"

struct poly_t;
typedef struct poly_t poly_t;

struct piece_t;
typedef struct piece_t piece_t;

void megaminx_corner(poly_t *mm, poly_t *dodec, float edge, int *facelets);
void megaminx_edge(poly_t *mm, poly_t *dodec, float edge, int* facelets);
void megaminx_centre(poly_t *mm, poly_t *dodec, float edge, int *facelets);

static const unsigned int megaminx_num_syms = 60;
typedef struct {
  quat *syms;
  unsigned int *by_vertex;
  unsigned int *by_edge;

  uint8_t *face_action;
} symmetries_t;

unsigned int symmetries_act_face(symmetries_t *syms, unsigned int f);

void gen_megaminx_syms(symmetries_t *syms, poly_t *dodec);

void megaminx_corner_piece(piece_t *piece, poly_t *corner,
                           symmetries_t *syms, int *facelets,
                           unsigned int i);
void megaminx_edge_piece(piece_t *piece, poly_t *edge,
                         symmetries_t *syms, int *facelets,
                         unsigned int e);
void megaminx_centre_piece(piece_t *piece, poly_t *centre,
                           symmetries_t *syms, int *facelets,
                           unsigned int f);

#endif /* MEGAMINX_H */
