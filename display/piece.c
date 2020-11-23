#include "piece.h"

#include <assert.h>
#include <GL/glew.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"
#include "polyhedron.h"
#include "shader.h"

#include "shaders/piece.v.glsl.h"
#include "shaders/piece.f.glsl.h"

vec3 *gen_vertices(poly_t *poly, unsigned int *num_vertices)
{
  *num_vertices = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    *num_vertices += poly->abs.faces[j].num_vertices;
  }

  vec3 *vertices = malloc(*num_vertices * sizeof(vec3));

  unsigned int index = 0;
  for (unsigned int i = 0; i < *num_vertices; i++) {
    memcpy(vertices[index++], poly->vertices[i], sizeof(vec3));
  }

  return vertices;
}

unsigned int *gen_elements(poly_t *poly, unsigned int *num_elements)
{
  *num_elements = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    *num_elements += (poly->abs.faces[j].num_vertices - 2) * 3;
  }

  unsigned int *elements = malloc(*num_elements * sizeof(unsigned int));

  unsigned int index = 0;
  for (unsigned int j = 0; j < poly->abs.num_faces; j++) {
    for (unsigned int i = 1; i < poly->abs.faces[j].num_vertices - 1; i++) {
      elements[index++] = poly->abs.faces[j].vertices[0];
      elements[index++] = poly->abs.faces[j].vertices[i];
      elements[index++] = poly->abs.faces[j].vertices[i + 1];

      printf("face: %u %u %u\n", elements[index - 3], elements[index - 2], elements[index - 1]);
    }
  }

  printf("index: %u, num_elements: %u\n", index, *num_elements);
  assert(index == *num_elements);
  return elements;
}

void piece_init(piece_t *piece, poly_t *poly)
{
  glGenVertexArrays(1, &piece->vao);
  glBindVertexArray(piece->vao);

  /* vertices */
  {
    unsigned int vbo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    unsigned int num_vertices;
    vec3 *vertices = gen_vertices(poly, &num_vertices);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(vec3), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    free(vertices);
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

  /* set up shaders */
  piece->shader = shader_load_program(piece_v_glsl, piece_v_glsl_len,
                                      piece_f_glsl, piece_f_glsl_len);
}

void piece_render(piece_t *piece, unsigned int width, unsigned int height)
{
  glBindVertexArray(piece->vao);
  glUseProgram(piece->shader);

  mat4x4 proj;
  mat4x4_perspective(proj, sqrt(2) * 0.5, (float) width / (float) height,
                     0.1f, 100.0f);
  mat4x4 view;
  mat4x4_translate(view, 0.0, 0.0, -15.0);
  mat4x4_rotate_X(view, view, 0.1);
  mat4x4_rotate_Y(view, view, 0.3);

  mat4x4 view_inv;
  mat4x4_invert(view_inv, view);

  {
    unsigned int var = glGetUniformLocation(piece->shader, "proj");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) proj);
  }
  {
    unsigned int var = glGetUniformLocation(piece->shader, "view");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) view);
    var = glGetUniformLocation(piece->shader, "view_inv");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) view_inv);
  }
  {
    mat4x4 m;
    mat4x4_identity(m);
    unsigned int var = glGetUniformLocation(piece->shader, "model");
    glUniformMatrix4fv(var, 1, GL_FALSE, (GLfloat *) m);
  }

  glDrawElementsInstanced(GL_TRIANGLES, piece->num_elements, GL_UNSIGNED_INT, 0, 1);
}
