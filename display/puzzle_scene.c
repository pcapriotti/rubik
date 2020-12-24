#include "puzzle_scene.h"

#include "piece.h"
#include "polyhedron.h"
#include "scene.h"

#include "lib/group.h"
#include "lib/puzzle.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

static void on_keypress(void *data, unsigned int c)
{
  puzzle_scene_t *s = data;
  if (c >= 256) return;

  if (c >= '0' && c <= '9') {
    unsigned int count = c - '0';
    s->count *= 10;
    s->count += count;
    return;
  }

  key_action_t *a = &s->key_bindings[c];
  if (a->run == 0) return;

  a->run(s, a->data);
  s->count = 0;
}

struct move_face_data_t
{
  unsigned int face;
  int count;
  int layer;
};

struct move_face_data_t *move_face_data_new(unsigned int face, int count, int layer)
{
  struct move_face_data_t *data = malloc(sizeof(struct move_face_data_t));
  data->face = face;
  data->count = count;
  data->layer = layer;
  return data;
}

void puzzle_scene_move_face(puzzle_scene_t *s, void *data_)
{
  struct move_face_data_t *data = data_;

  unsigned int layer = data->layer >= 0 ?
    (unsigned int) data->layer : s->count;
  turn_t *turn = s->puzzle->move(s->puzzle->move_data, s->conf,
                                 data->face, layer, data->count);
  if (!turn) return;

  const unsigned int num_orbits = s->model->decomp->num_orbits;
  unsigned int *num_pieces = malloc(num_orbits * sizeof(unsigned int));
  unsigned int **pieces = malloc(num_orbits * sizeof(unsigned int *));
  decomp_split_turn(s->model->decomp, turn, num_pieces, pieces);

  for (unsigned int k = 0; k < s->model->decomp->num_orbits; k++) {
    piece_turn(&s->piece[k], turn->g, num_pieces[k], pieces[k]);
  }

  free(num_pieces);
  free(pieces);
}

void puzzle_scene_rotate(puzzle_scene_t *s, void *data_)
{
  unsigned int *data = data_;
  unsigned int sym = *data;

  for (unsigned int i = 0; i < s->model->decomp->num_pieces; i++) {
    s->conf[i] = group_mul(s->puzzle->group, s->conf[i], sym);
  }

  for (unsigned int k = 0; k < s->model->decomp->num_orbits; k++) {
    piece_turn(&s->piece[k], sym, s->model->decomp->orbit_size[k], 0);
  }
}

void puzzle_scene_scramble(puzzle_scene_t *s, void *data)
{
  s->puzzle->scramble(s->puzzle->scramble_data, s->conf);
  for (unsigned int k = 0; k < s->model->decomp->num_orbits; k++) {
    piece_set_conf(&s->piece[k], s->conf + s->model->decomp->orbit_offset[k]);
  }
}

void puzzle_scene_init(puzzle_scene_t *s,
                       scene_t *scene,
                       uint8_t *conf,
                       puzzle_t *puzzle,
                       puzzle_model_t *model)
{
  s->count = 0;
  s->piece = malloc(puzzle->decomp->num_orbits * sizeof(piece_t));
  s->puzzle = puzzle;
  s->model = model;
  s->conf = conf;
  s->key_bindings = calloc(256, sizeof(key_action_t));

  for (unsigned int i = 0; i < model->decomp->num_orbits; i++) {
    poly_t poly;
    int *facelets = malloc(model->num_colours * sizeof(int));
    model->init_piece(model->init_piece_data, &poly, i,
                      model->orbit(model->orbit_data, i),
                      facelets);

    piece_init(&s->piece[i], &poly, facelets,
               model->decomp->orbit_offset[i],
               model->rots,
               conf + model->decomp->orbit_offset[i],
               model->decomp->orbit_size[i]);
    scene_add_piece(scene, &s->piece[i]);
    printf("adding orbit %u to scene\n", i);
    free(facelets);
  }

  /* colours */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 model->num_colours * sizeof(vec4),
                 model->colours, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_COLOURS, b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  /* facelet data */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    const unsigned int size = (1 + model->decomp->num_pieces *
                               model->num_colours) * sizeof(unsigned int);
    unsigned int *buf = malloc(size);
    buf[0] = model->num_colours;
    unsigned int index = 1;
    for (unsigned int k = 0; k < model->decomp->num_orbits; k++) {
      for (unsigned int x = 0; x < model->decomp->orbit_size[k]; x++) {
        for (unsigned int i = 0; i < model->num_colours; i++) {
          buf[index++] = model->facelet(model->facelet_data, k, x, i);
        }
      }
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, buf, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACELET, b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    free(buf);
  }

  s->key_bindings['s' - 'a' + 1] = (key_action_t) {
    .run = puzzle_scene_scramble,
    .data = 0
  };

  scene->on_keypress_data = s;
  scene->on_keypress = on_keypress;
}

void puzzle_scene_cleanup(puzzle_scene_t *s)
{
  for (unsigned int i = 0; i < 256; i++) {
    free(s->key_bindings[i].data);
  }
  free(s->key_bindings);
  s->puzzle->cleanup(s->puzzle->cleanup_data, s->puzzle);
  s->model->cleanup(s->model->cleanup_data, s->model);
}

void puzzle_scene_set_move_binding(puzzle_scene_t *s, unsigned char key,
                                   unsigned int f, int c, int l)
{
  s->key_bindings[key] = (key_action_t) {
    .run = puzzle_scene_move_face,
    .data = move_face_data_new(f, c, l)
  };
}

void puzzle_scene_set_rotation_binding(puzzle_scene_t *s, unsigned char key,
                                       unsigned int sym)
{
  unsigned int *data = malloc(sizeof(unsigned int));
  *data = sym;

  s->key_bindings[key] = (key_action_t) {
    .run = puzzle_scene_rotate,
    .data = data
  };
}
