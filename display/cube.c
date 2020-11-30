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
    facelets[i] = i;
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

void quat_cube_sym(quat q, uint8_t *perm, uint8_t s)
{
  uint8_t visited = 0;

  /* find positive cycle */
  int found = 0;

  vec3 axis;
  int sign;
  unsigned int length;
  unsigned int order = 1;

  for (unsigned int i = 0; i < 3; i++) {
    length = 0;
    sign = 1;
    if (!found) memset(axis, 0, sizeof(vec3));

    while (1) {
      uint8_t mask = 1 << i;
      if (visited & mask) {
        if (sign == 1 && length > 0) found = 1;

        unsigned int order0 = length;
        if (sign == -1) order0 *= 2;
        if (length > 0)
          order *= order0 / gcd(order0, order);

        break;
      }
      visited |= mask;
      sign *= (s & mask) ? -1 : 1;

      if (!found) {
        vec3 v = { 0, 0, 0 };
        v[i] = 1;

        vec3_scale(v, v, sign);
        vec3_add(axis, axis, v);
      }

      i = perm[i];
      length++;
    }
  }
  assert(found);

  vec3_norm(axis, axis);
  quat_rotate(q, 2 * M_PI / order, axis);
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

  unsigned int v = rotr3(e & 3, a);
  unsigned int v1 = cube_act_vertex(v, perm, s);

  return (a1 << 2) | (rotl3(v1, a) & 3);
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

  unsigned int index = 0;
  for (unsigned int p = 0; p < 6; p++) {
    uint8_t perm[3];
    perm_from_index(perm, 3, p, 3);
    uint8_t sign = perm_sign(perm, 3);

    for (unsigned int s = 0; s < 8; s++) {
       if (__builtin_popcount(s) % 2 != sign) continue;

      /* quaternions */
      quat_cube_sym(rots[index], perm, s);

      printf("[%u %u %u] %01x\n", perm[0], perm[1], perm[2], s);

      /* actions */
      printf("faces: ");
      for (unsigned int i = 0; i < cube->abs.num_faces; i++) {
        syms->face_action[i] = cube_act_face(i, perm, s);
        printf("%u ", syms->face_action[i]);
      }
      printf("\nvertices: ");
      for (unsigned int i = 0; i < cube->abs.num_vertices; i++) {
        syms->vertex_action[i] = cube_act_vertex(i, perm, s);
        printf("%u ", syms->vertex_action[i]);
      }
      printf("\nedges: ");
      for (unsigned int i = 0; i < num_edges; i++) {
        syms->edge_action[i] = cube_act_edge(i, perm, s);
        printf("%u ", syms->edge_action[i]);
      }
      printf("\n");
    }
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
  uint8_t pieces[] = { 0, 4, 8, 12 };

  piece_t *piece = malloc(sizeof(piece_t));
  piece_init(piece, &cube, facelets, pieces, 1);
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
                 sizeof(uint32_t) * cube_num_syms * 12,
                 buf, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACE_ACTION, b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }
}
