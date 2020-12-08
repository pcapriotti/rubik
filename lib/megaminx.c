#include "megaminx.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abs_poly.h"
#include "group.h"
#include "perm.h"
#include "puzzle.h"

static unsigned int smul(puzzle_t *puzzle, unsigned int a, unsigned int b)
{
  return group_mul(puzzle->group, a, b);
}

static unsigned int sconj(puzzle_t *puzzle, unsigned int a, unsigned int b)
{
  unsigned int b_inv_a = group_inv_mul(puzzle->group, b, a);
  return smul(puzzle, b_inv_a, b);
}

void megaminx_debug(puzzle_t *puzzle, megaminx_t *mm)
{
  for (unsigned int i = 0; i < puzzle->num_pieces; i++) {
    printf("piece %u symmetry %u position %u\n",
           i, mm->pieces[i],
           action_act(puzzle->action,
                      puzzle_repr(puzzle, i),
                      mm->pieces[i]));
  }
}

void megaminx_init(puzzle_t *puzzle, megaminx_t *mm)
{
  mm->pieces = malloc(puzzle->num_pieces);
  unsigned int index = 0;
  for (unsigned int k = 0; k < puzzle->num_orbits; k++) {
    for (unsigned int i = 0; i < puzzle->orbit_size[k]; i++) {
      mm->pieces[index++] = puzzle->by_stab[k][i];
    }
  }
}

void megaminx_cleanup(megaminx_t *mm)
{
  free(mm->pieces);
}

void megaminx_act_(puzzle_t *puzzle, megaminx_t *conf, megaminx_t *move)
{
  megaminx_act(puzzle, conf, conf, move);
}

void megaminx_act(puzzle_t *puzzle, megaminx_t *conf1,
                  megaminx_t *conf, megaminx_t *move)
{
  for (unsigned int i = 0; i < puzzle->num_pieces; i++) {
    unsigned int i1 = action_act(puzzle->action,
                                 puzzle_repr(puzzle, i),
                                 conf->pieces[i]);
    conf1->pieces[i] = group_mul(puzzle->group,
                                 conf->pieces[i],
                                 move->pieces[i1]);
  }
}

void megaminx_rotate_(puzzle_t *puzzle, megaminx_t *conf, unsigned int s)
{
  megaminx_rotate(puzzle, conf, conf, s);
}

void megaminx_rotate(puzzle_t *puzzle, megaminx_t *conf1,
                     megaminx_t *conf, unsigned int s)
{
  for (unsigned int i = 0; i < puzzle->num_pieces; i++) {
    conf1->pieces[i] = group_mul(puzzle->group, conf->pieces[i], s);
  }
}

void megaminx_mul(puzzle_t *puzzle, megaminx_t *ret,
                  megaminx_t *move1, megaminx_t *move2)
{
  for (unsigned int i = 0; i < puzzle->num_pieces; i++) {
    unsigned int i1 = action_act(puzzle->action, i, move1->pieces[i]);
    ret->pieces[i] = group_mul(puzzle->group, move1->pieces[i], move2->pieces[i1]);
  }
}

void megaminx_inv(puzzle_t *puzzle, megaminx_t *ret, megaminx_t *move)
{
  for (unsigned int i = 0; i < puzzle->num_pieces; i++) {
    unsigned int i1 = action_act(puzzle->action, i, move->pieces[i]);
    ret->pieces[i1] = group_inv(puzzle->group, move->pieces[i]);
  }
}

megaminx_t *megaminx_generators(puzzle_t *puzzle,
                                abs_poly_t *dodec,
                                poly_data_t *data,
                                unsigned int *num)
{
  *num = 12;
  megaminx_t *gen = malloc(*num * sizeof(megaminx_t));

  memset(gen, 0, sizeof(megaminx_t) * *num);
  for (unsigned int f = 0; f < *num; f++) {
    /* cw rotation around face f */
    unsigned int s = sconj(puzzle, 4, f * 5);
    gen[f].pieces = malloc(puzzle->num_pieces * sizeof(unsigned int));
    gen[f].pieces[puzzle_global(puzzle, 2, f)] = s;
    for (unsigned int i = 0; i < 5; i++) {
      gen[f].pieces[puzzle_global(puzzle, 0, dodec->faces[f].vertices[i])] = s;
      gen[f].pieces[puzzle_global(puzzle, 1, data->edges_by_face[f][i])] = s;
    }
  }

  return gen;
}

void even_shuffle(uint8_t *x, size_t len)
{
  shuffle(x, len);
  uint8_t s = perm_sign(x, len);
  if (s) {
    uint8_t tmp = x[0];
    x[0] = x[1];
    x[1] = tmp;
  }
}

void megaminx_scramble(puzzle_t *puzzle, megaminx_t *mm)
{
  for (unsigned int k = 0; k < 2; k++) {
    unsigned int stab_size = puzzle->group->num / puzzle->orbit_size[k];
    uint8_t *perm = malloc(puzzle->orbit_size[k]);
    perm_id(perm, puzzle->orbit_size[k]);
    even_shuffle(perm, puzzle->orbit_size[k]);

    unsigned int total = 0;
    for (unsigned int i = 0; i < puzzle->orbit_size[k]; i++) {
      unsigned int o;
      if (i == puzzle->orbit_size[k] - 1) {
        o = (stab_size - total % stab_size) % stab_size;
      }
      else {
        o = rand() % stab_size;
        total += o;
      }
      mm->pieces[puzzle_global(puzzle, 0, i)] =
        puzzle->by_stab[k][o * puzzle->orbit_size[k] + perm[i]];
    }

    free(perm);
  }
}

uint8_t *megaminx_orbit(puzzle_t *puzzle, megaminx_t *mm, unsigned int k)
{
  return &mm->pieces[puzzle->orbit_offset[k]];
}
