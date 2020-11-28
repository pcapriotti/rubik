#include "piece.h"

#include <assert.h>
#include <GL/glew.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"
#include "polyhedron.h"
#include "shader.h"

#include "shaders/piece.v.glsl.h"
#include "shaders/piece.f.glsl.h"

typedef struct
{
  vec3 vertex;
  vec3 normal;
  int facelet;
  vec3 bary;
} vdata_t;

static unsigned int shader = 0;

vdata_t *gen_vertices(poly_t *poly, unsigned int *num, int *facelets)
{
  *num = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    *num += poly->abs.faces[j].num_vertices * 3;
  }
  vdata_t *vdata = malloc(*num * sizeof(vdata_t));

  unsigned int index = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    unsigned int num = poly->abs.faces[j].num_vertices;
    /* compute face normal */
    vec3 a, b, n;
    vec3_sub(a, poly->vertices[poly->abs.faces[j].vertices[1]],
             poly->vertices[poly->abs.faces[j].vertices[0]]);
    vec3_sub(b, poly->vertices[poly->abs.faces[j].vertices[2]],
             poly->vertices[poly->abs.faces[j].vertices[1]]);

    vec3_mul_cross(n, a, b);
    vec3_norm(n, n);

    /* compute centre */
    vec3 centre = {0, 0, 0};
    for (unsigned int i = 0; i < num; i++) {
      vec3_add(centre, centre, poly->vertices[poly->abs.faces[j].vertices[i]]);
    }
    vec3_scale(centre, centre, 1.0 / (float) num);

    for (unsigned int i = 0; i < num; i++) {
      memcpy(vdata[index].vertex, centre, sizeof(vec3));
      memcpy(vdata[index].normal, n, sizeof(vec3));
      vdata[index].facelet = facelets[j];
      memcpy(vdata[index].bary, (vec3) {0, 0, 0}, sizeof(vec3));
      index++;

      memcpy(vdata[index].vertex, poly->vertices[poly->abs.faces[j].vertices[i]], sizeof(vec3));
      memcpy(vdata[index].normal, n, sizeof(vec3));
      vdata[index].facelet = facelets[j];
      memcpy(vdata[index].bary, (vec3) {0, 0, 0}, sizeof(vec3));
      index++;

      memcpy(vdata[index].vertex, poly->vertices[poly->abs.faces[j].vertices[(i + 1) % num]], sizeof(vec3));
      memcpy(vdata[index].normal, n, sizeof(vec3));
      vdata[index].facelet = facelets[j];
      memcpy(vdata[index].bary, (vec3) {0, 0, 0}, sizeof(vec3));
      index++;

      /* compute barycentric coordinates */
      for (unsigned int k = 0; k < 3; k++) {
        vec3 d, r;
        vec3_sub(d,
                 vdata[index - 3 + (k + 1) % 3].vertex,
                 vdata[index - 3 + (k + 2) % 3].vertex);
        vec3_mul_cross(r, n, d);
        vec3_norm(r, r);

        float value = vec3_mul_inner(vdata[index - 3 + (k + 1) % 3].vertex, r) -
          vec3_mul_inner(vdata[index - 3 + k].vertex, r);
        vdata[index - 3 + k].bary[k] = value;
      }
    }
  }

  return vdata;
}


unsigned int *gen_elements(poly_t *poly, unsigned int *num_elements)
{
  *num_elements = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    *num_elements += poly->abs.faces[j].num_vertices * 3;
  }

  unsigned int *elements = malloc(*num_elements * sizeof(unsigned int));

  unsigned int index = 0;
  unsigned int vindex = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    unsigned int n = poly->abs.faces[j].num_vertices;
    for (unsigned int i = 0; i < n; i++) {
      elements[index++] = vindex + i * 3;
      elements[index++] = vindex + i * 3 + 1;
      elements[index++] = vindex + i * 3 + 2;
    }

    vindex += poly->abs.faces[j].num_vertices * 3;
  }

  /* assert(index == *num_elements); */
  return elements;
}

void piece_init(piece_t *piece, poly_t *poly, int *facelets,
                uint8_t *conf, unsigned int instances)
{
  piece->instances = instances;

  glGenVertexArrays(1, &piece->vao);
  glBindVertexArray(piece->vao);

  /* vertices */
  {
    unsigned int vbo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    vdata_t *vdata = gen_vertices(poly, &piece->num_elements, facelets);
    glBufferData(GL_ARRAY_BUFFER,
                 piece->num_elements * sizeof(vdata_t),
                 vdata, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vdata_t), (void *) offsetof(vdata_t, vertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vdata_t), (void *) offsetof(vdata_t, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(2, 1, GL_INT,
                           sizeof(vdata_t), (void *) offsetof(vdata_t, facelet));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vdata_t), (void *) offsetof(vdata_t, bary));
    glEnableVertexAttribArray(3);

    free(vdata);
  }

  unsigned int *conf_buf = malloc(instances * sizeof(unsigned int));
  for (unsigned int i = 0; i < instances; i++) {
    conf_buf[i] = conf[i];
  }

  /* instance data */
  {
    glGenBuffers(1, &piece->sym_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, piece->sym_vbo);
    glBufferData(GL_ARRAY_BUFFER, instances * sizeof(unsigned int),
                 conf_buf, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, 0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
  }
  {
    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, instances * sizeof(unsigned int),
                 conf_buf, GL_STATIC_DRAW);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, 0, 0);
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);
  }

  free(conf_buf);

  /* set up shaders and uniforms */
  if (shader == 0)
    shader = shader_load_program(piece_v_glsl, piece_v_glsl_len,
                                 piece_f_glsl, piece_f_glsl_len);
}

void piece_set_conf(piece_t *piece, uint8_t *conf)
{
  unsigned int *buf = malloc(piece->instances * sizeof(unsigned int));
  for (unsigned int i = 0; i < piece->instances; i++) {
    buf[i] = conf[i];
  }
  glBindBuffer(GL_ARRAY_BUFFER, piece->sym_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  piece->instances * sizeof(unsigned int), buf);
  free(buf);
}

void piece_render(piece_t *piece)
{
  glBindVertexArray(piece->vao);
  glUseProgram(shader);
  glDrawArraysInstanced(GL_TRIANGLES, 0, piece->num_elements, piece->instances);
}
