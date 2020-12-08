#include "megaminx.h"
#include "piece.h"
#include "polyhedron.h"
#include "scene.h"
#include "utils.h"

#include "lib/abs_poly.h"
#include "lib/group.h"
#include "lib/megaminx.h"
#include "lib/puzzle.h"

#include <GL/glew.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* extract a megaminx corner from vertex 0 of a dodecahedron */
/* edge is the ratio between the corner edge length and the full edge length */
void megaminx_corner_poly(poly_t *mm, poly_t *dodec, float edge, int *facelets)
{
  abs_prism(&mm->abs, 4);

  vec3 p0, v1, v2, v3;
  memcpy(p0, dodec->vertices[0], sizeof(p0));

  vec3_sub(v1, dodec->vertices[1], p0); vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[4], p0); vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[5], p0); vec3_scale(v3, v3, edge);

  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));
  for (unsigned int i = 0; i < 8; i++) {
    memcpy(mm->vertices[i], p0, sizeof(vec3));
    if (i % 4 == 1 || i % 4 == 2) vec3_add(mm->vertices[i], mm->vertices[i], v1);
    if (i % 4 == 2 || i % 4 == 3) vec3_add(mm->vertices[i], mm->vertices[i], v2);
    if (i < 4) vec3_add(mm->vertices[i], mm->vertices[i], v3);
  }

  for (int i = 0; i < 6; i++) {
    facelets[i] = -1;
  }
  facelets[5] = 0;
  facelets[1] = 2;
  facelets[4] = 10;
}


void megaminx_edge_poly(poly_t *mm, poly_t *dodec, float edge, int* facelets)
{
  abs_prism(&mm->abs, 4);
  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));

  vec3 v1, v2, v3;
  vec3_sub(v1, dodec->vertices[1], dodec->vertices[0]);
  vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[4], dodec->vertices[0]);
  vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[5], dodec->vertices[0]);
  vec3_scale(v3, v3, edge);

  vec3_add(mm->vertices[0], dodec->vertices[0], v1);
  vec3_add(mm->vertices[1], mm->vertices[0], v3);
  vec3_add(mm->vertices[2], mm->vertices[1], v2);
  vec3_add(mm->vertices[3], mm->vertices[0], v2);

  vec3_sub(v1, dodec->vertices[0], dodec->vertices[1]);
  vec3_scale(v1, v1, edge);
  vec3_sub(v2, dodec->vertices[2], dodec->vertices[1]);
  vec3_scale(v2, v2, edge);
  vec3_sub(v3, dodec->vertices[6], dodec->vertices[1]);
  vec3_scale(v3, v3, edge);

  vec3_add(mm->vertices[4], dodec->vertices[1], v1);
  vec3_add(mm->vertices[5], mm->vertices[4], v3);
  vec3_add(mm->vertices[6], mm->vertices[5], v2);
  vec3_add(mm->vertices[7], mm->vertices[4], v2);

  for (int i = 0; i < 6; i++) {
    facelets[i] = -1;
  }
  facelets[1] = 2;
  facelets[4] = 0;
}

void megaminx_centre_poly(poly_t *mm, poly_t *dodec, float edge, int *facelets)
{
  abs_prism(&mm->abs, 5);
  mm->vertices = malloc(mm->abs.num_vertices * sizeof(vec3));

  for (unsigned int i = 0; i < 5; i++) {
    vec3 v1, v2, v3;
    vec3_sub(v1, dodec->vertices[(i + 1) % 5], dodec->vertices[i]);
    vec3_scale(v1, v1, edge);
    vec3_sub(v2, dodec->vertices[(i + 4) % 5], dodec->vertices[i]);
    vec3_scale(v2, v2, edge);
    vec3_sub(v3, dodec->vertices[i + 5], dodec->vertices[i]);
    vec3_scale(v3, v3, edge);

    vec3_add(mm->vertices[i + 5], dodec->vertices[i], v1);
    vec3_add(mm->vertices[i + 5], mm->vertices[i + 5], v2);
    vec3_add(mm->vertices[i], mm->vertices[i + 5], v3);
  }

  for (int i = 0; i < 10; i++) {
    facelets[i] = -1;
  }
  facelets[6] = 0;
}

quat *megaminx_rotations(poly_t *dodec)
{
  quat *rots = malloc(60 * sizeof(quat));

  /* symmetry i * 5 + j maps face 0 to i, and face 1 to the
  j-th face (in order from the lowest-numbered face) adjacent to i */

  quat rho;
  quat_rotate(rho, 2 * M_PI / 5, (vec3) { 0, 0, 1 });

  /* swap faces 0 and 2 */
  quat tau;
  rot_by_vertices(tau, dodec->vertices[0], dodec->vertices[1],
                  dodec->vertices[1], dodec->vertices[0]);

  quat_identity(rots[0]);
  /* map face 0 to 1 and 2 to 3 */
  rot_by_vertices(rots[5], dodec->vertices[0], dodec->vertices[1],
                  dodec->vertices[18], dodec->vertices[19]);

  for (unsigned int k = 0; k < 2; k++) {
    quat_mul(rots[(2 + k) * 5], rots[k * 5], tau);
    for (unsigned int j = 2; j <= 5; j++) {
      quat_mul(rots[(2 * j + k) * 5], rho, rots[(2 * (j - 1) + k) * 5]);
    }
  }

  for (unsigned int j = 0; j < 12; j++) {
    for (unsigned int i = 1; i < 5; i++) {
      quat_mul(rots[j * 5 + i], rots[j * 5 + i - 1], rho);
    }
  }

  return rots;
}

static void dodecahedron_group_init(group_t *group, abs_poly_t *dodec, poly_data_t *data)
{
  static const unsigned int num = 60;
  uint8_t *mul = malloc(num * num);

  for (unsigned int s = 0; s < num; s++) {
    uint8_t *table = &mul[num * s];

    unsigned int f0 = s / 5;
    unsigned int vi = s % 5;

    for (unsigned int j = 0; j < 5; j++) {
      table[j] = f0 * 5 + (vi + j) % 5;
      table[5 + j] = (f0 ^ 1) * 5 + (5 - vi + j) % 5;
    }

    int vi0 = data->adj[f0 * dodec->num_vertices + data->first_vertex[f0]];
    assert(vi0 != -1);
    for (unsigned int i = 0; i < 5; i++) {
      unsigned int v = dodec->faces[f0].vertices[(vi0 + vi + i) % 5];
      unsigned int w = dodec->faces[f0].vertices[(vi0 + vi + i + 1) % 5];
      unsigned int f1 = data->edges[w * dodec->num_vertices + v];

      int wi0 = data->adj[f1 * dodec->num_vertices + data->first_vertex[f1]];
      assert(wi0 != -1);
      int wi = data->adj[f1 * dodec->num_vertices + w];
      assert(wi != -1);

      for (unsigned int j = 0; j < 5; j++) {
        table[10 * i + 10 + j] = f1 * 5 + (wi - wi0 + j + 5) % 5;
        table[10 * i + 15 + j] = (f1 ^ 1) * 5 + (wi0 - wi + j + 5) % 5;
      }
    }
  }

  group_from_table(group, num, mul);
}

void megaminx_puzzle_init(puzzle_t *puzzle, abs_poly_t *dodec, poly_data_t *data)
{
  const unsigned int num_syms = 60;
  unsigned int orbit_size[] = { 20, 30, 12 };
  unsigned int stab_gen[] = { 10, 5, 1};

  group_t *group = malloc(sizeof(group_t));
  dodecahedron_group_init(group, dodec, data);

  uint8_t *orbit[3];
  uint8_t *stab[3];

  for (unsigned int k = 0; k < 3; k++) {
    orbit[k] = malloc(orbit_size[k]);
    memset(orbit[k], num_syms, orbit_size[k]);

    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* faces */
  for (unsigned int i = 0; i < orbit_size[2]; i++) {
    orbit[2][i] = i * 5;
  }

  /* vertices */
  {
    for (unsigned int j = 0; j < orbit_size[2]; j++) {
      unsigned int n = dodec->faces[j].num_vertices;
      for (unsigned int i = 0; i < n; i++) {
        unsigned int v = dodec->faces[j].vertices[i];
        unsigned int f = j;
        if (orbit[0][v] != num_syms) continue;
        int i0 = data->adj[f * dodec->num_vertices + data->first_vertex[f]];
        assert(i0 != -1);
        unsigned int s = f * 5 + (i - i0 + 5) % 5;
        orbit[0][v] = s;
      }
    }
  }

  /* edges */
  {
    unsigned int index = 0;
    for (unsigned int f = 0; f < dodec->num_faces; f++) {
      unsigned int n = dodec->faces[f].num_vertices;
      int vi0 = data->adj[f * dodec->num_vertices + data->first_vertex[f]];
      for (unsigned int i = 0; i < n; i++) {
        unsigned int f1 = abs_poly_get_adj_face(dodec, f, i, data->edges);
        if (f1 < f) continue;
        orbit[1][data->edges_by_face[f][i]] = f * 5 + (i - vi0 + 5) % 5;
      }
    }
  }

  puzzle_init(puzzle, 3, orbit_size, group, orbit, stab);

  for (unsigned int k = 0; k < 3; k++) {
    free(stab[k]);
    free(orbit[k]);
  }
}

void megaminx_piece_init(piece_t *piece, poly_t *poly, int *facelets,
                         unsigned int *sym_indices, unsigned int sym_stride,
                         unsigned int num)
{
  uint8_t *s = malloc(num * sizeof(uint8_t));
  for (unsigned int i = 0; i < num; i++) {
    s[i] = sym_indices[i * sym_stride];
  }
  piece_init(piece, poly, facelets, s, num);
  free(s);
}

struct key_action_t
{
  void (*run)(megaminx_scene_t *ms, void *data);
  void *data;
};
typedef struct key_action_t key_action_t;

struct megaminx_scene_t
{
  poly_t dodec;
  struct {
    poly_t poly;
    int facelets[6];
  } corner;

  struct {
    poly_t poly;
    int facelets[6];
  } edge;

  struct {
    poly_t poly;
    int facelets[7];
  } centre;

  megaminx_t mm;
  megaminx_t *gen;
  unsigned int num_gens;

  key_action_t *key_bindings;

  puzzle_t puzzle;
  quat *rots;
  piece_t piece[3];
};

void megaminx_scene_del(megaminx_scene_t *ms)
{
  puzzle_cleanup(&ms->puzzle);
  for (unsigned int i = 0; i < ms->num_gens; i++) {
    megaminx_cleanup(&ms->gen[i]);
  }
  free(ms->gen);
  megaminx_cleanup(&ms->mm);

  for (int i = 0; i < 3; i++) piece_cleanup(&ms->piece[i]);
  for (unsigned int i = 0; i < 256; i++) {
    free(ms->key_bindings[i].data);
  }
  free(ms->key_bindings);
  free(ms->rots);
  free(ms);
}

void megaminx_on_keypress(void *data, unsigned int c)
{
  megaminx_scene_t *ms = data;
  if (c >= 256) return;

  key_action_t *a = &ms->key_bindings[c];
  if (a->run == 0) return;

  a->run(ms, a->data);
}

struct move_face_data_t
{
  unsigned int face;
  unsigned int count;
};

static struct move_face_data_t *move_face_data_new(unsigned int face, unsigned int count)
{
  struct move_face_data_t *data = malloc(sizeof(struct move_face_data_t));
  data->face = face;
  data->count = count;
  return data;
}

void megaminx_action_move_face(megaminx_scene_t *ms, void *data_)
{
  struct move_face_data_t *data = data_;

  for (unsigned int i = 0; i < data->count; i++) {
    megaminx_act_(&ms->puzzle, &ms->mm, &ms->gen[data->face]);
  }

  for (unsigned int k = 0; k < 3; k++) {
    piece_set_conf(&ms->piece[k], megaminx_orbit(&ms->puzzle, &ms->mm, k));
  }
}

unsigned int *rotate_data_new(unsigned int s)
{
  unsigned int *data = malloc(sizeof(unsigned int));
  *data = s;
  return data;
}

void megaminx_action_rotate(megaminx_scene_t *ms, void *data_)
{
  unsigned int *data = data_;

  megaminx_rotate_(&ms->puzzle, &ms->mm, *data);

  for (unsigned int k = 0; k < 3; k++) {
    piece_set_conf(&ms->piece[k], megaminx_orbit(&ms->puzzle, &ms->mm, k));
  }
}

void megaminx_action_scramble(megaminx_scene_t *ms, void *data_)
{
  megaminx_scramble(&ms->puzzle, &ms->mm);

  for (unsigned int k = 0; k < 3; k++) {
    piece_set_conf_instant(&ms->piece[k], megaminx_orbit(&ms->puzzle, &ms->mm, k));
  }
}

void megaminx_scene_set_up_key_bindings(megaminx_scene_t *ms)
{
  ms->key_bindings = calloc(256, sizeof(key_action_t));

  static const unsigned char face_keys[] = "jfkdmv,c;als";
  static const unsigned char rot_keys[] = "JFKDMV<C/ALS";
  for (unsigned int i = 0; i < 12; i++) {
    ms->key_bindings[face_keys[i]] = (key_action_t) {
      .run = megaminx_action_move_face,
      .data = move_face_data_new(i & ~1, i & 1 ? 4 : 1)
    };
  }
  for (unsigned int i = 0; i < 12; i++) {
    unsigned int s = megaminx_orbit(&ms->puzzle, &ms->gen[i & ~1], 2)[i & ~1];
    if (i & 1) s = group_inv(ms->puzzle.group, s);
    ms->key_bindings[rot_keys[i]] = (key_action_t) {
      .run = megaminx_action_rotate,
      .data = rotate_data_new(s)
    };
  }

  ms->key_bindings['s' - 'a' + 1] = (key_action_t) {
    .run = megaminx_action_scramble,
    .data = 0
  };
}

megaminx_scene_t *megaminx_scene_new(scene_t *scene)
{
  static const float edge_size = 0.4;

  megaminx_scene_t *ms = malloc(sizeof(megaminx_scene_t));

  scene->on_keypress_data = ms;
  scene->on_keypress = megaminx_on_keypress;

  std_dodec(&ms->dodec);

  poly_data_t data;
  poly_data_init(&data, &ms->dodec.abs);

  megaminx_corner_poly(&ms->corner.poly, &ms->dodec, edge_size, ms->corner.facelets);
  megaminx_edge_poly(&ms->edge.poly, &ms->dodec, edge_size, ms->edge.facelets);
  megaminx_centre_poly(&ms->centre.poly, &ms->dodec, edge_size, ms->centre.facelets);
  ms->rots = megaminx_rotations(&ms->dodec);
  megaminx_puzzle_init(&ms->puzzle, &ms->dodec.abs, &data);

  /* symmetries */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(quat) * ms->puzzle.group->num,
                 ms->rots, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SYMS, b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  /* face action */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    const unsigned int size = sizeof(face_action_t) +
      ms->puzzle.group->num * 12 * sizeof(uint32_t);
    face_action_t *fa = malloc(size);
    fa->num_faces = 12;
    unsigned int index = 0;
    for (unsigned int g = 0; g < ms->puzzle.group->num; g++) {
      for (unsigned int f = 0; f < fa->num_faces; f++) {
        fa->action[index++] = puzzle_local
          (&ms->puzzle,
           action_act(ms->puzzle.action,
                      puzzle_global(&ms->puzzle, 2, f),
                      g));
      }
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, fa, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACE_ACTION, b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    free(fa);
  }

  /* colours */
  {
    vec4 colours[12] = {
      { 1.0, 1.0, 1.0, 1.0 }, // white
      { 0.4, 0.4, 0.4, 1.0 }, // grey
      { 1.0, 1.0, 0.0, 1.0 }, // yellow
      { 0.91, 0.85, 0.68, 1.0 }, // pale yellow
      { 0.5, 0.0, 0.5, 1.0 }, // purple
      { 1.0, 0.75, 0.8, 1.0 }, // pink
      { 0.0, 0.6, 0.0, 1.0 }, // green
      { 0.2, 0.8, 0.2, 1.0 }, // lime
      { 1.0, 0.0, 0.0, 1.0 }, // red
      { 1.0, 0.65, 0.0, 1.0 }, // orange
      { 0.0, 0.0, 1.0, 1.0 }, // blue
      { 0.0, 0.5, 0.5, 1.0 }, // teal
    };

    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * 12, colours, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_COLOURS, b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  megaminx_init(&ms->puzzle, &ms->mm);
  ms->gen = megaminx_generators(&ms->puzzle, &ms->dodec.abs, &data, &ms->num_gens);

  const unsigned int num_edges = ms->dodec.abs.num_faces +
    ms->dodec.abs.num_vertices - 2;
  piece_init(&ms->piece[0], &ms->corner.poly, ms->corner.facelets,
             megaminx_orbit(&ms->puzzle, &ms->mm, 0),
             ms->dodec.abs.num_vertices);
  piece_init(&ms->piece[1], &ms->edge.poly, ms->edge.facelets,
             megaminx_orbit(&ms->puzzle, &ms->mm, 1),
             num_edges);
  piece_init(&ms->piece[2], &ms->centre.poly, ms->centre.facelets,
             megaminx_orbit(&ms->puzzle, &ms->mm, 2),
             ms->dodec.abs.num_faces);

  scene_add_piece(scene, &ms->piece[0]);
  scene_add_piece(scene, &ms->piece[1]);
  scene_add_piece(scene, &ms->piece[2]);

  megaminx_scene_set_up_key_bindings(ms);

  poly_data_cleanup(&data);

  return ms;
}
