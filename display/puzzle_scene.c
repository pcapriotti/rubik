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
  free(s->key_bindings);
  s->puzzle->cleanup(s->puzzle->cleanup_data, s->puzzle);
  s->model->cleanup(s->model->cleanup_data, s->model);
}
