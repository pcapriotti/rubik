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
} vdata_t;

vdata_t *gen_vertices(poly_t *poly, unsigned int *num, int *facelets)
{
  *num = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    *num += poly->abs.faces[j].num_vertices;
  }
  vdata_t *vdata = malloc(*num * sizeof(vdata_t));

  unsigned int index = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    /* compute face normal */
    vec3 a, b, n;
    vec3_sub(a, poly->vertices[poly->abs.faces[j].vertices[1]],
             poly->vertices[poly->abs.faces[j].vertices[0]]);
    vec3_sub(b, poly->vertices[poly->abs.faces[j].vertices[2]],
             poly->vertices[poly->abs.faces[j].vertices[1]]);

    vec3_mul_cross(n, a, b);
    vec3_norm(n, n);

    for (unsigned int i = 0; i < poly->abs.faces[j].num_vertices; i++) {
      memcpy(vdata[index].vertex, poly->vertices[poly->abs.faces[j].vertices[i]], sizeof(vec3));
      memcpy(vdata[index].normal, n, sizeof(vec3));
      vdata[index].facelet = facelets[j];
      index++;
    }
  }

  return vdata;
}


unsigned int *gen_elements(poly_t *poly, unsigned int *num_elements)
{
  *num_elements = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    *num_elements += (poly->abs.faces[j].num_vertices - 2) * 3;
  }

  unsigned int *elements = malloc(*num_elements * sizeof(unsigned int));

  unsigned int index = 0;
  unsigned int vindex = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    for (unsigned int i = 1; i < poly->abs.faces[j].num_vertices - 1; i++) {
      elements[index++] = vindex;
      elements[index++] = vindex + i;
      elements[index++] = vindex + i + 1;
    }

    vindex += poly->abs.faces[j].num_vertices;
  }

  /* assert(index == *num_elements); */
  return elements;
}

void piece_init(piece_t *piece, poly_t *poly, int *facelets)
{
  mat4x4_identity(piece->model);

  glGenVertexArrays(1, &piece->vao);
  glBindVertexArray(piece->vao);

  /* vertices */
  {
    unsigned int vbo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    unsigned int num;
    vdata_t *vdata = gen_vertices(poly, &num, facelets);
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(vdata_t), vdata, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vdata_t), (void *) offsetof(vdata_t, vertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vdata_t), (void *) offsetof(vdata_t, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(vdata_t), (void *) offsetof(vdata_t, facelet));
    glEnableVertexAttribArray(2);

    free(vdata);
  }

  /* elements */
  {
    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    unsigned int *elements = gen_elements(poly, &piece->num_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, piece->num_elements * sizeof(unsigned int),
                 elements, GL_STATIC_DRAW);
    free(elements);
  }

  /* set up shaders and uniforms */
  piece->shader = shader_load_program(piece_v_glsl, piece_v_glsl_len,
                                      piece_f_glsl, piece_f_glsl_len);
}

void piece_update(piece_t *piece, mat4x4 proj,
                  mat4x4 view, mat4x4 view_inv,
                  mat4x4 model, vec3 lpos)
{
  glUseProgram(piece->shader);
  {
    unsigned int var = glGetUniformLocation(piece->shader, "view");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) view);
    var = glGetUniformLocation(piece->shader, "view_inv");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) view_inv);
  }
  {
    mat4x4 m;
    mat4x4_mul(m, model, piece->model);
    unsigned int var = glGetUniformLocation(piece->shader, "model");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) m);
  }
  {
    unsigned int var = glGetUniformLocation(piece->shader, "lpos");
    glUniform3fv(var, 1, lpos);
  }
  {
    unsigned int var = glGetUniformLocation(piece->shader, "proj");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) proj);
  }
}

void piece_render(piece_t *piece)
{
  glBindVertexArray(piece->vao);
  glUseProgram(piece->shader);

  glDrawElementsInstanced(GL_TRIANGLES, piece->num_elements, GL_UNSIGNED_INT, 0, 1);
}
