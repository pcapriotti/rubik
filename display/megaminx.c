#include "megaminx.h"
#include "piece.h"
#include "polyhedron.h"
#include "scene.h"

#include "lib/megaminx.h"

#include <GL/glew.h>
#include <assert.h>
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

/* return symmetry mapping a pair of adjacent vertices to another one */
void rot_by_vertices(quat q, vec3 v1, vec3 v2, vec3 w1, vec3 w2)
{
  /* q1 maps v1 to w1 */
  quat q1;
  vec3 a1;
  vec3_mul_cross(a1, v1, w1);
  vec3_norm(a1, a1);
  quat_rotate(q1, acos(vec3_mul_inner(v1, w1)), a1);

  vec3 v2_;
  quat_mul_vec3(v2_, q1, v2);

  /* q2 maps v2_ to w2 and fixes w1 */
  quat q2;
  vec3 a2; vec3_mul_cross(a2, v2_, w2);
  float p1 = vec3_mul_inner(v2_, w1);
  float p2 = vec3_mul_inner(w2, w1);
  float d = (vec3_mul_inner(v2_, w2) - p1 * p2) /
    (sqrtf(1 - p1 * p1) * sqrtf(1 - p2 * p2));
  float angle = vec3_mul_inner(a2, w1) > 0 ? acos(d) : -acos(d);
  quat_rotate(q2, angle, w1);

  quat_mul(q, q2, q1);

  #if 1
  {
    vec3 x;
    quat_mul_vec3(x, q, v1);
    vec3_sub(x, x, w1);
    assert(vec3_len(x) <= 0.001);

    quat_mul_vec3(x, q, v2);
    vec3_sub(x, x, w2);
    assert(vec3_len(x) <= 0.001);
  }
  #endif
}

quat *megaminx_syms_init(symmetries_t *syms, poly_t *dodec)
{
  /* symmetry i * 5 + j maps face 0 to i, and face 1 to the
  j-th face (in order from the lowest-numbered face) adjacent to i */

  quat *rots = malloc(megaminx_num_syms * sizeof(quat));
  syms->by_vertex = malloc(megaminx_num_syms * sizeof(unsigned int));
  syms->by_edge = malloc(megaminx_num_syms * sizeof(unsigned int));
  syms->edges_by_face = malloc(dodec->abs.num_faces * 5 * sizeof(unsigned int));

  const unsigned int num_edges = dodec->abs.num_faces +
    dodec->abs.num_vertices - 2;
  syms->face_action = malloc(megaminx_num_syms *
                             dodec->abs.num_faces *
                             sizeof(uint32_t));
  syms->vertex_action = malloc(megaminx_num_syms *
                               dodec->abs.num_vertices *
                               sizeof(uint8_t));
  syms->edge_action = malloc(megaminx_num_syms *
                             num_edges *
                             sizeof(uint8_t));

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

  int *edges = abs_poly_edges(&dodec->abs);
  int *v0 = abs_poly_first_vertex(&dodec->abs, edges);
  int *adj = abs_poly_adj(&dodec->abs);

  /* by_vertex */
  {
    for (unsigned int i = 0; i < megaminx_num_syms; i++) {
      syms->by_vertex[i] = megaminx_num_syms;
    }
    for (unsigned int j = 0; j < dodec->abs.num_faces; j++) {
      unsigned int n = dodec->abs.faces[j].num_vertices;
      for (unsigned int i = 0; i < n; i++) {
        unsigned int v = dodec->abs.faces[j].vertices[i];
        unsigned int f = j;
        int vi = i;
        if (syms->by_vertex[v * 3] != megaminx_num_syms) continue;

        for (unsigned int k = 0; k < 3; k++) {
          int vi0 = adj[f * dodec->abs.num_vertices + v0[f]];
          assert(vi0 != -1);
          unsigned int s = f * 5 + (vi - vi0 + 5) % 5;
          syms->by_vertex[v * 3 + k] = s;

          f = abs_poly_get_adj_face(&dodec->abs, f, vi - 1, edges);
          assert(f < dodec->abs.num_faces);
          vi = adj[f * dodec->abs.num_vertices + v];
        }
      }
    }
  }

  /* by_edge */
  {
    for (unsigned int i = 0; i < megaminx_num_syms; i++) {
      syms->by_edge[i] = megaminx_num_syms;
    }
    unsigned int index = 0;
    unsigned int *edge_index = calloc(sizeof(unsigned int),
                                      dodec->abs.num_faces * 5);
    for (unsigned int f = 0; f < dodec->abs.num_faces; f++) {
      unsigned int n = dodec->abs.faces[f].num_vertices;
      int vi0 = adj[f * dodec->abs.num_vertices + v0[f]];
      for (unsigned int i = 0; i < n; i++) {
        unsigned int v = dodec->abs.faces[f].vertices[i];
        unsigned int f1 = abs_poly_get_adj_face(&dodec->abs, f, i, edges);
        if (f1 < f) continue;
        int i1 = adj[f1 * dodec->abs.num_vertices + v];
        assert(i1 != -1);
        i1 -= 1;
        int vi1 = adj[f1 * dodec->abs.num_vertices + v0[f1]];

        syms->edges_by_face[f * 5 + edge_index[f]++] = index / 2;
        syms->edges_by_face[f1 * 5 + edge_index[f1]++] = index / 2;
        syms->by_edge[index++] = f * 5 + (i - vi0 + 5) % 5;
        syms->by_edge[index++] = f1 * 5 + (i1 - vi1 + 5) % 5;
      }
    }

    free(edge_index);
  }

  /* construct multiplication table */
  syms->mul = malloc(megaminx_num_syms * megaminx_num_syms * sizeof(uint8_t));
  for (unsigned int s = 0; s < megaminx_num_syms; s++) {
    uint8_t *table = &syms->mul[megaminx_num_syms * s];

    unsigned int f0 = s / 5;
    unsigned int vi = s % 5;

    for (unsigned int j = 0; j < 5; j++) {
      table[j] = f0 * 5 + (vi + j) % 5;
      table[5 + j] = (f0 ^ 1) * 5 + (5 - vi + j) % 5;
    }

    int vi0 = adj[f0 * dodec->abs.num_vertices + v0[f0]];
    assert(vi0 != -1);
    for (unsigned int i = 0; i < 5; i++) {
      unsigned int v = dodec->abs.faces[f0].vertices[(vi0 + vi + i) % 5];
      unsigned int w = dodec->abs.faces[f0].vertices[(vi0 + vi + i + 1) % 5];
      unsigned int f1 = edges[w * dodec->abs.num_vertices + v];

      int wi0 = adj[f1 * dodec->abs.num_vertices + v0[f1]];
      assert(wi0 != -1);
      int wi = adj[f1 * dodec->abs.num_vertices + w];
      assert(wi != -1);

      for (unsigned int j = 0; j < 5; j++) {
        table[10 * i + 10 + j] = f1 * 5 + (wi - wi0 + j + 5) % 5;
        table[10 * i + 15 + j] = (f1 ^ 1) * 5 + (wi0 - wi + j + 5) % 5;
      }
    }
  }

  /* inverse multiplication table */
  syms->inv_mul = malloc(megaminx_num_syms * megaminx_num_syms * sizeof(uint8_t));
  for (unsigned int a = 0; a < megaminx_num_syms; a++) {
    for (unsigned int b = 0; b < megaminx_num_syms; b++) {
      unsigned int c = syms->mul[megaminx_num_syms * b + a];
      syms->inv_mul[megaminx_num_syms * c + a] = b;
    }
  }

  /* face action */
  for (unsigned int s = 0; s < megaminx_num_syms; s++) {
    for (unsigned int i = 0; i < dodec->abs.num_faces; i++) {
      syms->face_action[dodec->abs.num_faces * s + i] =
        syms->mul[megaminx_num_syms * s + i * 5] / 5;
    }
  }
  /* action on vertex 0 */
  for (unsigned int i = 0; i < dodec->abs.num_vertices; i++) {
    for (unsigned int j = 0; j < 3; j++) {
      unsigned int s = syms->by_vertex[i * 3 + j];
      syms->vertex_action[dodec->abs.num_vertices * s] = i;
    }
  }
  /* vertex action */
  for (unsigned int s = 0; s < megaminx_num_syms; s++) {
    for (unsigned int i = 1; i < dodec->abs.num_vertices; i++) {
      syms->vertex_action[dodec->abs.num_vertices * s + i] =
        syms->vertex_action[dodec->abs.num_vertices *
                            syms->mul[megaminx_num_syms * s +
                                      syms->by_vertex[i * 3]]];
    }
  }
  /* action on edge 0 */
  for (unsigned int i = 0; i < num_edges; i++) {
    for (unsigned int j = 0; j < 2; j++) {
      unsigned int s = syms->by_edge[i * 2 + j];
      syms->edge_action[num_edges * s] = i;
    }
  }
  /* edge action */
  for (unsigned int s = 0; s < megaminx_num_syms; s++) {
    for (unsigned int i = 1; i < num_edges; i++) {
      syms->edge_action[num_edges * s + i] =
        syms->edge_action[num_edges *
                          syms->mul[megaminx_num_syms * s +
                                    syms->by_edge[i * 2]]];
    }
  }


  free(adj);
  free(v0);
  free(edges);

  return rots;
}

void megaminx_syms_cleanup(symmetries_t *syms)
{
  free(syms->by_vertex);
  free(syms->by_edge);
  free(syms->edges_by_face);
  free(syms->face_action);
  free(syms->vertex_action);
  free(syms->edge_action);
  free(syms->mul);
  free(syms->inv_mul);
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

struct action_t
{
  void (*run)(megaminx_scene_t *ms, void *data);
  void *data;
};
typedef struct action_t action_t;

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

  action_t *key_bindings;

  symmetries_t syms;
  quat *rots;
  piece_t piece[3];
};

void megaminx_scene_del(megaminx_scene_t *ms)
{
  megaminx_syms_cleanup(&ms->syms);
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

  action_t *a = &ms->key_bindings[c];
  if (a->run == 0) return;

  a->run(ms, a->data);
}

struct move_face_data_t
{
  unsigned int face;
  unsigned int count;
};

struct move_face_data_t *move_face_data_new(unsigned int face, unsigned int count)
{
  struct move_face_data_t *data = malloc(sizeof(struct move_face_data_t));
  data->face = face;
  data->count = count;
  return data;
}

void megaminx_action_move_face(megaminx_scene_t *ms, void *data_)
{
  struct move_face_data_t *data = data_;

  printf("moving:\n");
  megaminx_debug(&ms->syms, &ms->gen[data->face]);
  printf("---\nbefore:\n");
  megaminx_debug(&ms->syms, &ms->mm);

  for (unsigned int i = 0; i < data->count; i++) {
    megaminx_act_(&ms->syms, &ms->mm, &ms->gen[data->face]);
  }

  printf("---\nafter:\n");
  megaminx_debug(&ms->syms, &ms->mm);

  piece_set_conf(&ms->piece[0], megaminx_corner(&ms->mm, 0));
  piece_set_conf(&ms->piece[1], megaminx_edge(&ms->mm, 0));
  piece_set_conf(&ms->piece[2], megaminx_centre(&ms->mm, 0));
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

  megaminx_rotate_(&ms->syms, &ms->mm, *data);

  piece_set_conf(&ms->piece[0], megaminx_corner(&ms->mm, 0));
  piece_set_conf(&ms->piece[1], megaminx_edge(&ms->mm, 0));
  piece_set_conf(&ms->piece[2], megaminx_centre(&ms->mm, 0));
}

void megaminx_scene_set_up_key_bindings(megaminx_scene_t *ms)
{
  ms->key_bindings = calloc(256, sizeof(action_t));

  /* face moves */
#define BIND(x, f, c) \
  ms->key_bindings[x] = (action_t) { \
    .run = megaminx_action_move_face, \
    .data = move_face_data_new(f, c) }
  BIND('j', 0, 1);
  BIND('k', 2, 1);
  BIND('l', 10, 1);
  BIND(';', 8, 1);
  BIND('m', 4, 1);
  BIND(',', 6, 1);
  BIND('f', 0, 4);
  BIND('d', 2, 4);
  BIND('s', 10, 4);
  BIND('a', 8, 4);
  BIND('v', 4, 4);
  BIND('c', 6, 4);
#undef BIND

  ms->key_bindings['K'] = (action_t) { \
    .run = megaminx_action_rotate, \
    .data = rotate_data_new(21) };
  ms->key_bindings['D'] = (action_t) { \
    .run = megaminx_action_rotate, \
    .data = rotate_data_new(11) };
}

megaminx_scene_t *megaminx_scene_new(scene_t *scene)
{
  static const float edge_size = 0.4;

  megaminx_scene_t *ms = malloc(sizeof(megaminx_scene_t));

  scene->on_keypress_data = ms;
  scene->on_keypress = megaminx_on_keypress;

  std_dodec(&ms->dodec);
  megaminx_corner_poly(&ms->corner.poly, &ms->dodec, edge_size, ms->corner.facelets);
  megaminx_edge_poly(&ms->edge.poly, &ms->dodec, edge_size, ms->edge.facelets);
  megaminx_centre_poly(&ms->centre.poly, &ms->dodec, edge_size, ms->centre.facelets);
  ms->rots = megaminx_syms_init(&ms->syms, &ms->dodec);

  /* symmetries */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(quat) * megaminx_num_syms,
                 ms->rots, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SYMS, b);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  /* face action */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(uint32_t) * megaminx_num_syms * 12,
                 ms->syms.face_action, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACE_ACTION, b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  megaminx_init(&ms->syms, &ms->mm);
  ms->gen = megaminx_generators(&ms->syms, &ms->dodec.abs, &ms->num_gens);

  const unsigned int num_edges = ms->dodec.abs.num_faces +
    ms->dodec.abs.num_vertices - 2;
  piece_init(&ms->piece[0], &ms->corner.poly, ms->corner.facelets,
             megaminx_corner(&ms->mm, 0),
             ms->dodec.abs.num_vertices);
  piece_init(&ms->piece[1], &ms->edge.poly, ms->edge.facelets,
             megaminx_edge(&ms->mm, 0),
             num_edges);
  piece_init(&ms->piece[2], &ms->centre.poly, ms->centre.facelets,
             megaminx_centre(&ms->mm, 0),
             ms->dodec.abs.num_faces);

  scene_add_piece(scene, &ms->piece[0]);
  scene_add_piece(scene, &ms->piece[1]);
  scene_add_piece(scene, &ms->piece[2]);

  megaminx_scene_set_up_key_bindings(ms);

  return ms;
}
