#include "polyhedron.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void poly_debug(poly_t *poly)
{
  abs_poly_debug(&poly->abs);
  for (unsigned int i = 0; i < poly->abs.num_vertices; i++) {
    printf("(%.2f, %.2f, %.2f) ", poly->vertices[i][0], poly->vertices[i][1], poly->vertices[i][2]);
  }
  printf("\n");
}

void std_cube(poly_t *poly)
{
  abs_cube(&poly->abs);

  poly->vertices = malloc(poly->abs.num_vertices * sizeof(vec3));

  for (unsigned int n = 0; n < poly->abs.num_vertices; n++) {
    for (unsigned int i = 0; i < 3; i++) {
      poly->vertices[n][i] = (float)((n >> i) & 1) - 0.5f;
    }
  }
}

void std_prism(poly_t *poly, unsigned int num)
{
  abs_prism(&poly->abs, num);
  poly->vertices = malloc(poly->abs.num_vertices * sizeof(vec3));

  mat4x4 rho;
  mat4x4_identity(rho);
  mat4x4_rotate_Z(rho, rho, 2 * M_PI / num);

  vec4 v0 = { 1, 0, -1, 1 };

  for (unsigned int i = 0; i < num; i++) {
    vec4 v;
    memcpy(poly->vertices[i], v0, sizeof(vec3));
    memcpy(poly->vertices[i + num], v0, sizeof(vec3));
    poly->vertices[i + num][2] = 1;

    memcpy(v, v0, sizeof(v));
    mat4x4_mul_vec4(v0, rho, v);
  }
}

void std_dodec(poly_t *poly)
{
  abs_dodec(&poly->abs);
  poly->vertices = malloc(poly->abs.num_vertices * sizeof(vec3));

  float phi = (sqrt(5) + 1) / 2.0;
  float r = 2.0 / (sqrt(3) * phi * sqrt(3 - phi));
  float z = r * (phi + 1) / 2.0;

  mat4x4 rho;
  mat4x4_identity(rho);
  mat4x4_rotate_Z(rho, rho, 2 * M_PI / 5);

  vec4 v0 = { r, 0, z, 1};
  vec4 v5 = { r * phi, 0, z - r, 1 };

  for (unsigned int i = 0; i < 5; i++) {
    vec4 v;

    memcpy(poly->vertices[i], v0, sizeof(vec3));
    memcpy(v, v0, sizeof(v));
    mat4x4_mul_vec4(v0, rho, v);

    memcpy(poly->vertices[i + 5], v5, sizeof(vec3));
    memcpy(v, v5, sizeof(v));
    mat4x4_mul_vec4(v5, rho, v);

    vec3_scale(poly->vertices[19 - i], poly->vertices[i], -1);
    vec3_scale(poly->vertices[14 - i], poly->vertices[i + 5], -1);
  }
}

