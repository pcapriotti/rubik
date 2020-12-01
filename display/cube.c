#include "cube.h"

#include <GL/glew.h>
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"
#include "piece.h"
#include "polyhedron.h"
#include "scene.h"

#include "lib/cube.h"
#include "lib/group.h"
#include "lib/perm.h"
#include "lib/puzzle.h"
#include "lib/utils.h"

void cube_piece_poly(poly_t *cube, float edge, int *facelets)
{
  std_cube(cube);
  mat4x4 m;
  mat4x4_identity(m);
  mat4x4_scale_aniso(m, m, edge, edge, edge);
  memcpy(m[3], cube->vertices[0], sizeof(vec3));
  vec3_scale(m[3], m[3], 1 - edge);

  for (unsigned int i = 0; i < cube->abs.num_vertices; i++) {
    vec4 v, w;
    memcpy(v, cube->vertices[i], sizeof(vec3));
    v[3] = 1.0;
    mat4x4_mul_vec4(w, m, v);
    memcpy(cube->vertices[i], w, sizeof(vec3));
  }

  for (unsigned int i = 0; i < 6; i++) {
    facelets[i] = -1;
  }
}

int gcd(int m, int n)
{
  while (m) {
    int x = m;
    m = n % m;
    n = x;
  }
  return n;
}

void quat_from_transp(quat q, unsigned int a, unsigned int b)
{
  memset(q, 0, sizeof(quat));
  if (a != b) {
    q[a] = q[b] = 1;
  }
  else {
    q[3] = 1;
  }
}

void quat_from_perm(quat q, uint8_t *perm)
{
  /* transpose 0 and perm[0] */
  quat q0;
  quat_from_transp(q0, 0, perm[0]);
  printf("q0: (%f, %f, %f, %f)\n", q0[0], q0[1], q0[2], q0[3]);

  uint8_t perm1[3];
  perm_id(perm1, 3);
  perm1[0] = perm[0];
  perm1[perm[0]] = 0;
  perm_mul(perm1, perm, 3);

  printf("perm1: [%u %u %u]\n", perm1[0], perm1[1], perm1[2]);

  /* transpose 1 and 2 */
  quat q1;
  quat_from_transp(q1, 1, perm1[1]);
  printf("q1: (%f, %f, %f, %f)\n", q1[0], q1[1], q1[2], q1[3]);

  quat_mul(q, q0, q1);
  printf("q: (%f, %f, %f, %f)\n", q[0], q[1], q[2], q[3]);
}

void quat_cube_sym(quat q, uint8_t s)
{
  for (unsigned int i = 0; i < 3; i++) {
    if (s & (1 << i)) {
      quat q1;
      memset(q1, 0, sizeof(quat));
      q1[i] = 1;

      quat tmp; memcpy(tmp, q, sizeof(quat));
      quat_mul(q, tmp, q1);
    }
  }
  quat_norm(q, q);
  printf("q: (%f, %f, %f, %f)\n", q[0], q[1], q[2], q[3]);
}

unsigned int cube_act_face(unsigned int f, uint8_t *perm, unsigned int s)
{
  unsigned int i = f >> 1;
  unsigned int f1 = (perm[i] << 1) | (f & 1);
  if (s & (1 << i)) f1 ^= 1;
  return f1;
}

unsigned int cube_act_vertex(unsigned int v, uint8_t *perm, unsigned int s)
{
  v = v ^ s;
  unsigned int v1 = 0;
  for (unsigned int i = 0; i < 3; i++) {
    if (v & (1 << i)) v1 |= (1 << perm[i]);
  }
  return v1;
}

unsigned int cube_act_edge(unsigned int e, uint8_t *perm, unsigned int s)
{
  unsigned int a = e >> 2;
  unsigned int a1 = perm[a];

  unsigned int v = rotl3(e & 3, (a + 1) % 3);
  unsigned int v1 = cube_act_vertex(v, perm, s);

  return (a1 << 2) | (rotr3(v1, (a1 + 1) % 3) & 3);
}

void cube_mul_table(uint8_t *table, uint8_t *perm1, unsigned int s1)
{
  unsigned int index = 0;
  uint8_t perm2[3];
  for (unsigned int p1 = 0; p1 < 6; p1++) {
    perm_from_index(perm2, 3, p1, 3);
    unsigned int sign = perm_sign(perm2, 3);

    for (unsigned int j = 0; j < 4; j++) {
      unsigned int s2 = j | ((sign ^ (__builtin_popcount(j) & 1)) << 2);

      uint8_t perm[3];
      perm_composed(perm, perm1, perm2, 3);

      table[index++] = (perm_index(perm, 3, 3) << 2) |
        ((s2 ^ u16_conj(s1, perm2, 3)) & 0x3);

      /* printf("perm1: [%u %u %u], perm2: [%u %u %u] ", */
      /*        perm1[0], perm1[1], perm1[2], */
      /*        perm2[0], perm2[1], perm2[2]); */
      /* printf("s1: %01x s2: %01x s2': %01x, index: %u, res: %u\n", */
      /*        s1, s2, u16_conj(s2, perm1, 3), */
      /*        perm_index(perm2, 3, 3), */
      /*        table[index - 1]); */
    }
  }
}

quat *cube_syms_init(symmetries_t *syms, poly_t *cube)
{
  quat *rots = malloc(cube_num_syms * sizeof(quat));
  syms->face_action = malloc(cube_num_syms * cube->abs.num_faces *
                             sizeof(uint8_t));
  syms->vertex_action = malloc(cube_num_syms * cube->abs.num_vertices *
                               sizeof(uint8_t));
  const unsigned int num_edges = cube->abs.num_faces + cube->abs.num_vertices - 2;
  syms->edge_action = malloc(cube_num_syms * num_edges * sizeof(uint8_t));
  syms->mul = malloc(cube_num_syms * cube_num_syms * sizeof(uint8_t));
  syms->inv_mul = malloc(cube_num_syms * cube_num_syms * sizeof(uint8_t));
  syms->by_face = malloc(cube_num_syms * sizeof(unsigned int));
  syms->by_vertex = malloc(cube_num_syms * sizeof(unsigned int));
  syms->by_edge = malloc(cube_num_syms * sizeof(unsigned int));

  unsigned int index = 0;
  for (unsigned int p = 0; p < 6; p++) {
    uint8_t lehmer[3];
    uint8_t perm[3];

    lehmer_from_index(lehmer, 3, p, 3);
    perm_from_lehmer(perm, lehmer, 3);
    uint8_t sign = lehmer_sign(lehmer, 3);

    /* quaternion correponding to the element of Pin(3) determined by
    this permutation */
    quat q0;
    quat_from_perm(q0, perm);

    for (unsigned int j = 0; j < 4; j++) {
      unsigned int s = j | ((sign ^ (__builtin_popcount(j) & 1)) << 2);

      printf("[%u %u %u] %01x\n", perm[0], perm[1], perm[2], s);
      printf("q0: (%f, %f, %f, %f)\n", q0[0], q0[1], q0[2], q0[3]);

      /* set quaternion */
      memcpy(rots[index], q0, sizeof(quat));
      quat_cube_sym(rots[index], s);

      /* actions */
      printf("faces: ");
      for (unsigned int i = 0; i < cube->abs.num_faces; i++) {
        syms->face_action[index * cube->abs.num_faces + i] =
          cube_act_face(i, perm, s);
        printf("%u ", syms->face_action[index * cube->abs.num_faces + i]);
      }
      printf("\nvertices: ");
      for (unsigned int i = 0; i < cube->abs.num_vertices; i++) {
        syms->vertex_action[index * cube->abs.num_vertices + i] =
          cube_act_vertex(i, perm, s);
        printf("%u ", syms->vertex_action[index * cube->abs.num_vertices + i]);
      }
      printf("\nedges: ");
      for (unsigned int i = 0; i < num_edges; i++) {
        syms->edge_action[index * num_edges + i] =
          cube_act_edge(i, perm, s);
        printf("%u ", syms->edge_action[index * num_edges + i]);
      }
      printf("\n");

      /* multiplication table */
      cube_mul_table(&syms->mul[index * cube_num_syms], perm, s);

      index++;
    }
  }

  group_inv_table(syms->inv_mul, syms->mul, cube_num_syms);

  /* by face */
  static const int face_stab = 4;
  for (unsigned int f = 0; f < 6; f++) {
    uint8_t sign = (f >> 1) & 1;
    unsigned int s = f & 1;
    s = s | ((sign ^ s) << 2);
    unsigned int index = ((f & ~1) << 2) | s;

    syms->by_face[f * 4] = index;
    for (unsigned int i = 1; i < 4; i++) {
      syms->by_face[f * 4 + i] = syms->mul
        [syms->by_face[f * 4 + i - 1] * cube_num_syms + face_stab];
    }
  }

  /* by vertex */
  static const int vertex_stab = 12;
  for (unsigned int v = 0; v < 8; v++) {
    unsigned int p = __builtin_popcount(v) & 1;
    unsigned int s = (v & 1) | ((v >> p) & 2);
    unsigned int index = (p << 2) | s;
    syms->by_vertex[v * 3] = index;
    for (unsigned int i = 1; i < 3; i++) {
      syms->by_vertex[v * 3 + i] = syms->mul
        [syms->by_vertex[v * 3 + i - 1] * cube_num_syms + vertex_stab];
    }
  }

  /* by edge */
  static const int edge_stab = 5;
  for (unsigned int e = 0; e < 12; e++) {
    unsigned int a = e >> 2;
    unsigned int p = a & 1;
    unsigned int s = ((e & 1) << p) | ((e & 2) >> p);
    s = ((s & 1) << 1) | ((__builtin_popcount(s) & 1) ^ p);
    unsigned int index = (a << 3) | s;
    syms->by_edge[e * 2] = index;
    syms->by_edge[e * 2 + 1] = syms->mul
      [syms->by_edge[e * 2] * cube_num_syms + edge_stab];
  }

  return rots;
}

void cube_scene_new(scene_t *scene, unsigned int n)
{
  int facelets[6];
  poly_t cube;
  cube_piece_poly(&cube, 1.0 / (float) n, facelets);
  poly_debug(&cube);

  symmetries_t syms;
  quat *rots = cube_syms_init(&syms, &cube);

  /* cube_t conf; */
  /* cube_init(&syms, &conf, n); */
  uint8_t conf[2] = {0, 4};

  piece_t *piece = malloc(sizeof(piece_t));
  piece_init(piece, &cube, facelets, conf, 2);
  scene_add_piece(scene, piece);

  /* symmetries */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(quat) * cube_num_syms,
                 rots, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SYMS, b);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  /* face action */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    uint32_t *buf = malloc(cube_num_syms * 6 * sizeof(uint32_t));
    for (unsigned int i = 0; i < cube_num_syms * 6; i++) {
      buf[i] = syms.face_action[i];
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(uint32_t) * cube_num_syms * 6,
                 buf, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACE_ACTION, b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }
}
