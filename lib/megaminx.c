#include "megaminx.h"

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

void megaminx_init(symmetries_t *syms, megaminx_t *mm)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    mm->corners[i] = syms->by_vertex[i * 3];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_EDGES; i++) {
    mm->edges[i] = syms->by_edge[i * 2];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_CENTRES; i++) {
    mm->centres[i] = i * 5;
  }
}

void megaminx_act_(symmetries_t *syms, megaminx_t *conf, megaminx_t *move)
{
  megaminx_act(syms, conf, conf, move);
}

void megaminx_act(symmetries_t *syms, megaminx_t *conf1,
                  megaminx_t *conf, megaminx_t *move)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    unsigned int i1 = syms->vertex_action[conf->corners[i] * megaminx_num_syms];
    conf1->corners[i] = smul(syms, conf->corners[i], move->corners[i1]);
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_EDGES; i++) {
    unsigned int i1 = syms->edge_action[conf->edges[i] * megaminx_num_syms];
    conf1->edges[i] = smul(syms, conf->edges[i], move->edges[i1]);
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_CENTRES; i++) {
    unsigned int i1 = syms->face_action[conf->centres[i] * megaminx_num_syms];
    conf1->centres[i] = smul(syms, conf->centres[i], move->corners[i1]);
  }
}

void megaminx_mul(symmetries_t *syms, megaminx_t *ret,
                  megaminx_t *move1, megaminx_t *move2)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    unsigned int i1 = syms->vertex_action
      [move1->corners[i] * megaminx_num_syms + i];
    ret->corners[i] = smul(syms, move1->corners[i], move2->corners[i1]);
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_EDGES; i++) {
    unsigned int i1 = syms->edge_action
      [move1->edges[i] * megaminx_num_syms + i];
    ret->edges[i] = smul(syms, move1->edges[i], move2->edges[i1]);
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_CENTRES; i++) {
    unsigned int i1 = syms->face_action
      [move1->centres[i] * megaminx_num_syms + i];
    ret->centres[i] = smul(syms, move1->centres[i], move2->corners[i1]);
  }
}

void megaminx_inv(symmetries_t *syms, megaminx_t *ret, megaminx_t *move)
{
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    unsigned int i1 = syms->vertex_action
      [move->corners[i] * megaminx_num_syms + i];
    ret->corners[i1] = syms->inv_mul[move->corners[i]];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_EDGES; i++) {
    unsigned int i1 = syms->edge_action
      [move->edges[i] * megaminx_num_syms + i];
    ret->edges[i1] = syms->inv_mul[move->edges[i]];
  }
  for (unsigned int i = 0; i < MEGAMINX_NUM_CORNERS; i++) {
    unsigned int i1 = syms->face_action
      [move->centres[i] * megaminx_num_syms + i];
    ret->centres[i1] = syms->inv_mul[move->centres[i]];
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
    gen[f].centres[f] = s;
    for (unsigned int i = 0; i < 5; i++) {
      gen[f].corners[dodec->faces[f].vertices[i]] = s;
      gen[f].edges[syms->edges_by_face[f * 5 + i]] = s;
    }
  }

  return gen;
}
