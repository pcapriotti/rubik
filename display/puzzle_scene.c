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
};

struct move_face_data_t *move_face_data_new(unsigned int face, int count)
{
  struct move_face_data_t *data = malloc(sizeof(struct move_face_data_t));
  data->face = face;
  data->count = count;
  return data;
}

void puzzle_scene_move_face(puzzle_scene_t *s, void *data_)
{
  struct move_face_data_t *data = data_;

  printf("moving face: %u layer: %u count: %d\n",
         data->face, s->count, data->count);
  turn_t *turn = s->puzzle->move(s->puzzle->move_data, s->conf,
                                 data->face, s->count, data->count);
  if (!turn) return;

  const unsigned int num_orbits = s->puzzle->decomp->num_orbits;
  unsigned int *num_pieces = malloc(num_orbits * sizeof(unsigned int));
  unsigned int **pieces = malloc(num_orbits * sizeof(unsigned int *));
  decomp_split_turn(s->puzzle->decomp, turn, num_pieces, pieces);

  for (unsigned int k = 0; k < s->puzzle->decomp->num_orbits; k++) {
    piece_turn(&s->piece[k], turn->g, num_pieces[k], pieces[k]);
  }

  free(num_pieces);
  free(pieces);
}

void puzzle_scene_rotate(puzzle_scene_t *s, void *data_)
{
  unsigned int *data = data_;
  unsigned int sym = *data;

  for (unsigned int i = 0; i < s->puzzle->decomp->num_pieces; i++) {
    s->conf[i] = group_mul(s->puzzle->group, s->conf[i], sym);
  }

  for (unsigned int k = 0; k < s->puzzle->decomp->num_orbits; k++) {
    piece_turn(&s->piece[k], sym, s->puzzle->decomp->orbit_size[k], 0);
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

  for (unsigned int i = 0; i < puzzle->decomp->num_orbits; i++) {
    poly_t poly;
    int *facelets = malloc(puzzle->num_faces * sizeof(int));
    model->init_piece(model->init_piece_data, &poly, i,
                      puzzle->orbit(puzzle->orbit_data, i),
                      facelets);
    piece_init(&s->piece[i], &poly, facelets, model->rots,
               conf + puzzle->decomp->orbit_offset[i],
               puzzle->decomp->orbit_size[i]);
    scene_add_piece(scene, &s->piece[i]);
    printf("adding orbit %u to scene\n", i);
    free(facelets);
  }

  /* face action */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    const unsigned int size = sizeof(face_action_t) +
      puzzle->group->num * puzzle->num_faces * sizeof(uint32_t);
    face_action_t *fa = malloc(size);
    fa->num_faces = puzzle->num_faces;
    unsigned int index = 0;
    for (unsigned int g = 0; g < puzzle->group->num; g++) {
      for (unsigned int f = 0; f < fa->num_faces; f++) {
        fa->action[index++] = puzzle->face_action(puzzle->face_action_data, f, g);
      }
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, fa, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACE_ACTION, b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    free(fa);
  }

  /* colours */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 puzzle->num_faces * sizeof(vec4),
                 model->colours, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_COLOURS, b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

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
                                   unsigned int f, int c)
{
  s->key_bindings[key] = (key_action_t) {
    .run = puzzle_scene_move_face,
    .data = move_face_data_new(f, c)
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
