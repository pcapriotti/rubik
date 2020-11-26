#include "megaminx.h"
#include "polyhedron.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* extract a megaminx corner from vertex 0 of a dodecahedron */
/* edge is the ratio between the corner edge length and the full edge length */
void megaminx_corner(poly_t *mm, poly_t *dodec, float edge)
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
}


void megaminx_edge(poly_t *mm, poly_t *dodec, float edge)
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
}

void megaminx_centre(poly_t *mm, poly_t *dodec, float edge)
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

void gen_megaminx_syms(symmetries_t *syms, poly_t *dodec)
{
  /* symmetry i * 5 + j maps face 0 to i, and face 1 to the
  j-th face (in order from the lowest-numbered face) adjacent to i */

  syms->syms = malloc(megaminx_num_syms * sizeof(quat));
  syms->by_vertex = malloc(megaminx_num_syms * sizeof(unsigned int));
  syms->by_edge = malloc(megaminx_num_syms * sizeof(unsigned int));

  const unsigned int num_edges = dodec->abs.num_faces +
    dodec->abs.num_vertices - 2;
  syms->face_action = malloc(megaminx_num_syms *
                             dodec->abs.num_faces *
                             sizeof(unsigned int));
  /* syms->vertex_action = malloc(megaminx_num_syms * */
  /*                              dodec->abs.num_vertices * */
  /*                              sizeof(unsigned int)); */
  /* syms->edge_action = malloc(megaminx_num_syms * */
  /*                            num_edges * */
  /*                            sizeof(unsigned int)); */

  quat rho;
  quat_rotate(rho, 2 * M_PI / 5, (vec3) { 0, 0, 1 });

  /* swap faces 0 and 2 */
  quat tau;
  rot_by_vertices(tau, dodec->vertices[0], dodec->vertices[1],
                  dodec->vertices[1], dodec->vertices[0]);

  quat_identity(syms->syms[0]);
  /* map face 0 to 1 and 2 to 3 */
  rot_by_vertices(syms->syms[5], dodec->vertices[0], dodec->vertices[1],
                  dodec->vertices[18], dodec->vertices[19]);

  for (unsigned int k = 0; k < 2; k++) {
    quat_mul(syms->syms[(2 + k) * 5], syms->syms[k * 5], tau);
    for (unsigned int j = 2; j <= 5; j++) {
      quat_mul(syms->syms[(2 * j + k) * 5], rho, syms->syms[(2 * (j - 1) + k) * 5]);
    }
  }

  for (unsigned int j = 0; j < 12; j++) {
    for (unsigned int i = 1; i < 5; i++) {
      quat_mul(syms->syms[j * 5 + i], syms->syms[j * 5 + i - 1], rho);
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

        syms->by_edge[index++] = f * 5 + (i - vi0 + 5) % 5;
        syms->by_edge[index++] = f1 * 5 + (i1 - vi1 + 5) % 5;
      }
    }
  }

  /* actions */
  for (unsigned int s = 0; s < megaminx_num_syms; s++) {
    uint8_t action[12];
    unsigned int f0 = s / 5;
    unsigned int vi = s % 5;

    action[0] = f0;
    action[1] = f0 ^ 1;
    int vi0 = adj[f0 * dodec->abs.num_vertices + v0[f0]];
    for (unsigned int i = 1; i <= 5; i++) {
      action[2 * i] = abs_poly_get_adj_face(&dodec->abs, f0, vi0 + i, edges);
      action[2 * i + 1] = action[2 * i + 1] ^ 1;
    }
  }

  free(adj);
  free(v0);
  free(edges);
}
