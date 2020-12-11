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

void cube_piece_poly(poly_t *cube, unsigned int n,
                     unsigned int x, unsigned int y, unsigned int z,
                     int *facelets)
{
  std_cube(cube);
  mat4x4 m;

  float side = -2.0 * cube->vertices[0][0];
  float edge = 1 / (float) n;
  float gap = 0.03;
  float edge1 = (1 + gap) * edge;

  vec3 offset = {
    side * edge1 * (x - ((float) n - 1) / 2),
    side * edge1 * (y - ((float) n - 1) / 2),
    side * edge1 * (z - ((float) n - 1) / 2),
  };
  mat4x4_translate(m, offset[0], offset[1], offset[2]);
  mat4x4_scale_aniso(m, m, edge, edge, edge);

  for (unsigned int i = 0; i < cube->abs.num_vertices; i++) {
    vec4 v, w;
    memcpy(v, cube->vertices[i], sizeof(vec3));
    v[3] = 1.0;
    mat4x4_mul_vec4(w, m, v);
    memcpy(cube->vertices[i], w, sizeof(vec3));
  }

  unsigned int coords[3] = {x, y, z};
  for (unsigned int i = 0; i < 6; i++) {
    facelets[i] = (coords[i / 2] == (~i & 1) * (n - 1)) ? (int) i : -1;
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
    q[a] = 1;
    q[b] = -1;
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

  uint8_t perm1[3];
  perm_id(perm1, 3);
  perm1[0] = perm[0];
  perm1[perm[0]] = 0;
  perm_mul(perm1, perm, 3);

  /* transpose 1 and 2 */
  quat q1;
  quat_from_transp(q1, 1, perm1[1]);

  quat_mul(q, q0, q1);
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
  /* printf("q: (%f, %f, %f, %f)\n", q[0], q[1], q[2], q[3]); */
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

quat *cube_puzzle_init(puzzle_t *puzzle)
{
  static const unsigned int num_syms = 24;

  /* The group G' of symmetries of a cube is the wreath product G' of
  Sigma_3 and O(1). G' acts linearly on the cube by permuting
  coordinates and changing signs. Explicitly, a pair (sigma, u) of a
  permutation and a choice of signs acts on the basis elements as
  follows:

    e_i (sigma, u) = u_i e_(sigma i).

  The determinant of (sigma, u) is sign(sigma) * sum(u). We are
  interested in the subgroup G of G' consisting of those elements that
  have determinant 1.

  They can be enumerated by enumerating the permutation sigma first,
  then choosing all possible signs for the first two axes, and setting
  the third sign to the only value that makes the determinant
  positive.

  To generate quaternions for all possible elements of G, we regard a
  quaternion as an element of Spin(3), and observe that Pin(3) is the
  direct product Spin(3) x O(1). Therefore, an element of Pin(3) can
  be represented by a pair of a quaternion and a sign. The strategy is
  then to write every element of G as a product of Pin(3) elements
  corresponding to reflections (i.e. tensors of rank 1 in the Clifford
  algebra).

  Since we know that this product is ultimately going to be
  a rotation, we can ignore the sign component, and simply use a
  quaternion to represent a reflection, namely the reflection
  corresponding to the composition of the usual rotation with scaling
  by -1 (i.e. reflection with respect to the origin).

  With these conventions, a reflection along a vector v corresponds to
  v itself regarded as a tensor, i.e. the purely imaginary quaternion
  associated to v.
 */

  group_t *group = malloc(sizeof(group_t));
  uint8_t *mul = malloc(num_syms * num_syms);
  quat *rots = malloc(num_syms * sizeof(quat));

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

      cube_mul_table(&mul[index * num_syms], perm, s);

      /* printf("[%u %u %u] %01x\n", perm[0], perm[1], perm[2], s); */
      /* printf("perm: (%f, %f, %f, %f)\n", q0[0], q0[1], q0[2], q0[3]); */

      memcpy(rots[index], q0, sizeof(quat));
      quat_cube_sym(rots[index], s);
      index++;
    }
  }

  group_from_table(group, num_syms, mul);

  unsigned int orbit_size[3] = { 8, 12, 6 };
  unsigned int stab_gen[3] = { 12, 5, 4 };

  uint8_t *stab[3];
  uint8_t *orbit[3];

  for (unsigned int k = 0; k < 3; k++) {
    orbit[k] = malloc(orbit_size[k]);
    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* vertices */
  for (unsigned int v = 0; v < 8; v++) {
    unsigned int p = __builtin_popcount(v) & 1;
    unsigned int s = (v & 1) | ((v >> p)& 2);
    orbit[0][v] = (p << 2) | s;
  }

  /* edges */
  for (unsigned int e = 0; e < 12; e++) {
    unsigned int a = e >> 2;
    unsigned int p = a & 1;
    unsigned int s = ((e & 1) << p) | ((e & 2) >> p);
    s = ((s & 1) << 1) | ((__builtin_popcount(s) & 1) ^ p);
    orbit[1][e] = (a << 3) | s;
  }

  /* faces */
  for (unsigned int f = 0; f < 6; f++) {
    uint8_t sign = (f >> 1) & 1;
    unsigned int s = f & 1;
    orbit[2][f] = ((f & ~1) << 2) | s;
  }

  puzzle_init(puzzle, 3, orbit_size, group, orbit, stab);

  for (unsigned int k = 0; k < 3; k++) {
    free(stab[k]);
    free(orbit[k]);
  }

  return rots;
}

struct key_action_t
{
  void (*run)(cube_scene_t *ms, void *data);
  void *data;
};
typedef struct key_action_t key_action_t;

struct cube_scene_t
{
  key_action_t *key_bindings;
  puzzle_t puzzle;
  cube_t conf;
  piece_t *piece;

  unsigned int count;
};

void cube_scene_cleanup(cube_scene_t *s)
{
  free(s->key_bindings);
  puzzle_cleanup(&s->puzzle);

  cube_shape_t *shape = s->conf.shape;
  cube_cleanup(&s->conf);
  cube_shape_cleanup(shape);
}

struct move_face_data_t
{
  unsigned int face;
  int count;
};

static struct move_face_data_t *move_face_data_new(unsigned int face, int count)
{
  struct move_face_data_t *data = malloc(sizeof(struct move_face_data_t));
  data->face = face;
  data->count = count;
  return data;
}

void cube_action_move_face(cube_scene_t *s, void *data_)
{
  struct move_face_data_t *data = data_;

  printf("moving face: %u layer: %u count: %d\n",
         data->face, s->count, data->count);
  turn_t *turn = cube_move_(&s->puzzle, &s->conf, data->face, s->count, data->count);

  unsigned int *num_pieces = malloc(turn->num_pieces * sizeof(unsigned int));
  unsigned int **pieces = malloc(turn->num_pieces * sizeof(unsigned int *));
  decomp_split_turn(&s->conf.shape->decomp, turn, num_pieces, pieces);

  for (unsigned int k = 0; k < s->conf.shape->decomp.num_orbits; k++) {
    piece_turn(&s->piece[k], turn->g, num_pieces[k], pieces[k]);
  }

}

void cube_scene_set_up_key_bindings(cube_scene_t *s)
{
  s->key_bindings = calloc(256, sizeof(key_action_t));

  static const unsigned char face_keys[] = "jfmvkd,cls;a";
  static const unsigned char rot_keys[] = "JFMVKD<CLS:A";
  for (unsigned int i = 0; i < 12; i++) {
    s->key_bindings[face_keys[i]] = (key_action_t) {
      .run = cube_action_move_face,
      .data = move_face_data_new(i >> 1, (i & 1) ? 1 : -1)
    };
  }
}

static void cube_on_keypress(void *data, unsigned int c)
{
  cube_scene_t *s = data;
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

cube_scene_t *cube_scene_new(scene_t *scene, unsigned int n)
{
  cube_scene_t *s = malloc(sizeof(cube_scene_t));
  s->count = 0;

  quat *rots = cube_puzzle_init(&s->puzzle);

  cube_shape_t *shape = malloc(sizeof(cube_shape_t));
  cube_shape_init(shape, n);
  cube_init(&s->puzzle, &s->conf, shape);

  s->piece = malloc(s->conf.shape->decomp.num_orbits * sizeof(piece_t));
  for (unsigned int i = 0; i < s->conf.shape->decomp.num_orbits; i++) {
    int facelets[6];
    poly_t cube;
    orbit_t *orbit = &s->conf.shape->orbits[i];
    cube_piece_poly(&cube, n, orbit->x, orbit->y, orbit->z, facelets);

    piece_init(&s->piece[i], &cube, facelets, rots,
               cube_orbit(&s->conf, i),
               s->conf.shape->decomp.orbit_size[i]);
    scene_add_piece(scene, &s->piece[i]);

    /* printf("added orbit %u, dim: %u, size: %u, pos: (%u, %u, %u)\n", */
    /*        i, orbit->dim, orbit->size, orbit->x, orbit->y, orbit->z); */
  }

  /* face action */
  {
    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);

    const unsigned int size = sizeof(face_action_t) +
      cube_num_syms * 6 * sizeof(uint32_t);
    face_action_t *fa = malloc(size);
    fa->num_faces = 6;
    unsigned int index = 0;
    for (unsigned int g = 0; g < s->puzzle.group->num; g++) {
      for (unsigned int f = 0; f < fa->num_faces; f++) {
        fa->action[index++] = decomp_local
          (&s->puzzle.decomp,
           puzzle_act(&s->puzzle,
                      decomp_global(&s->puzzle.decomp, 2, f),
                      g));
      }
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, fa, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_FACE_ACTION, b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    free(fa);
  }

  /* colours */
  {
    vec4 colours[] = {
      { 1.0, 1.0, 1.0, 1.0 }, // white
      { 1.0, 1.0, 0.0, 1.0 }, // yellow
      { 0.0, 0.6, 0.0, 1.0 }, // green
      { 0.0, 0.0, 1.0, 1.0 }, // blue
      { 1.0, 0.0, 0.0, 1.0 }, // red
      { 1.0, 0.65, 0.0, 1.0 }, // orange
    };

    unsigned int b;
    glGenBuffers(1, &b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_COLOURS, b);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  cube_scene_set_up_key_bindings(s);

  scene->on_keypress_data = s;
  scene->on_keypress = cube_on_keypress;

  return s;
}
