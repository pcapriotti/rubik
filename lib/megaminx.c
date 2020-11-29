#include "megaminx.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abs_poly.h"

unsigned int smul(symmetries_t *syms, unsigned int a, unsigned int b)
{
  return syms->mul[a + b * megaminx_num_syms];
}

unsigned int sconj(symmetries_t *syms, unsigned int a, unsigned int b)
{
  unsigned int b_inv_a = syms->inv_mul[b + a * megaminx_num_syms];
  return smul(syms, b_inv_a, b);
}

unsigned int act(symmetries_t *syms, unsigned int x, unsigned int s)
{
  if (x < MEGAMINX_NUM_CORNERS) {
    return syms->vertex_action[MEGAMINX_NUM_CORNERS * s + x];
  }
  x -= MEGAMINX_NUM_CORNERS;

  if (x < MEGAMINX_NUM_EDGES) {
    return MEGAMINX_NUM_CORNERS +
      syms->edge_action[MEGAMINX_NUM_EDGES * s + x];
  }
  x -= MEGAMINX_NUM_EDGES;

  assert(x < MEGAMINX_NUM_CENTRES);
  return MEGAMINX_NUM_CORNERS + MEGAMINX_NUM_EDGES +
    syms->face_action[MEGAMINX_NUM_CENTRES * s + x];
}

unsigned int repr(unsigned int x)
{
  if (x < MEGAMINX_NUM_CORNERS) return 0;
  if (x < MEGAMINX_NUM_CORNERS + MEGAMINX_NUM_EDGES) return MEGAMINX_NUM_CORNERS;
  return MEGAMINX_NUM_CORNERS + MEGAMINX_NUM_EDGES;
}

uint8_t *megaminx_corner(megaminx_t *mm, unsigned int c)
{
  return &mm->pieces[c];
}

uint8_t *megaminx_edge(megaminx_t *mm, unsigned int e)
{
  return &mm->pieces[e + MEGAMINX_NUM_CORNERS];
}

uint8_t *megaminx_centre(megaminx_t *mm, unsigned int f)
{
  return &mm->pieces[f + MEGAMINX_NUM_CORNERS + MEGAMINX_NUM_EDGES];
}

void megaminx_debug(symmetries_t *syms, megaminx_t *mm)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_PIECES; i++) {
    printf("piece %u symmetry %u position %u\n",
           i, mm->pieces[i], act(syms, repr(i), mm->pieces[i]));
  }
}

void megaminx_init(symmetries_t *syms, megaminx_t *mm)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    mm->pieces[i] = syms->by_vertex[i * 3];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_EDGES; i++) {
    mm->pieces[i + MEGAMINX_NUM_CORNERS] = syms->by_edge[i * 2];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_CENTRES; i++) {
    mm->pieces[i + MEGAMINX_NUM_CORNERS + MEGAMINX_NUM_EDGES] = i * 5;
  }
}

void megaminx_act_(symmetries_t *syms, megaminx_t *conf, megaminx_t *move)
{
  megaminx_act(syms, conf, conf, move);
}

void megaminx_act(symmetries_t *syms, megaminx_t *conf1,
                  megaminx_t *conf, megaminx_t *move)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_PIECES; i++) {
    unsigned int i1 = act(syms, repr(i), conf->pieces[i]);
    conf1->pieces[i] = smul(syms, conf->pieces[i], move->pieces[i1]);
  }
}

void megaminx_rotate(symmetries_t *syms, megaminx_t *conf1,
                     megaminx_t *conf, unsigned int s)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_PIECES; i++) {
    conf1->pieces[i] = smul(syms, conf->pieces[i], s);
  }
}

void megaminx_rotate_(symmetries_t *syms, megaminx_t *conf, unsigned int s)
{
  megaminx_rotate(syms, conf, conf, s);
}

void megaminx_mul(symmetries_t *syms, megaminx_t *ret,
                  megaminx_t *move1, megaminx_t *move2)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_PIECES; i++) {
    unsigned int i1 = act(syms, i, move1->pieces[i]);
    ret->pieces[i] = smul(syms, move1->pieces[i], move2->pieces[i1]);
  }
}

void megaminx_inv(symmetries_t *syms, megaminx_t *ret, megaminx_t *move)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_PIECES; i++) {
    unsigned int i1 = act(syms, i, move->pieces[i]);
    ret->pieces[i1] = syms->inv_mul[move->pieces[i]];
  }
}

void megaminx_conj(symmetries_t *syms, megaminx_t *ret,
                   megaminx_t *move1, megaminx_t *move2)
{
  megaminx_t move2_inv, tmp;
  megaminx_inv(syms, &move2_inv, move2);

  megaminx_mul(syms, &tmp, &move2_inv, move1);
  megaminx_mul(syms, ret, &tmp, move2);
}

megaminx_t *megaminx_generators(symmetries_t *syms,
                                abs_poly_t *dodec,
                                unsigned int *num)
{
  *num = 12;
  megaminx_t *gen = malloc(*num * sizeof(megaminx_t));

  memset(gen, 0, sizeof(megaminx_t) * *num);
  for (unsigned int f = 0; f < *num; f++) {
    /* cw rotation around face f */
    unsigned int s = sconj(syms, 4, f * 5);
    *megaminx_centre(&gen[f], f) = s;
    for (unsigned int i = 0; i < 5; i++) {
      *megaminx_corner(&gen[f], dodec->faces[f].vertices[i]) = s;
      *megaminx_edge(&gen[f], syms->edges_by_face[f * 5 + i]) = s;
    }
  }

  return gen;
}
