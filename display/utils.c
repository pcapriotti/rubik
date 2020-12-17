#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SLERP_EPS 0.0001

void die(const char *msg)
{
  perror(msg);
  exit(1);
}

int category(char c)
{
  if (isspace(c)) return CATEGORY_SPACE;
  if (isalnum(c) || c == '_') return CATEGORY_ALPHANUM;
  return CATEGORY_OTHER;
}

void skip_category(const char *s, int len, int cat, int direction, int *x)
{
  while (*x >= 0 && *x < len && category(s[*x]) == cat) {
    *x += direction;
  }
}

/* rotation mapping v to w and fixing axis */
void rot_by_axis_and_vertices(quat q, vec3 axis, vec3 v, vec3 w)
{
  printf("axis: (%.02f, %.02f, %.02f)\n", axis[0], axis[1], axis[2]);
  printf("v: (%.02f, %.02f, %.02f)\n", v[0], v[1], v[2]);
  printf("w: (%.02f, %.02f, %.02f)\n", w[0], w[1], w[2]);
  vec3 a2; vec3_mul_cross(a2, v, w);
  float p1 = vec3_mul_inner(v, axis);
  float p2 = vec3_mul_inner(w, axis);
  float d = (vec3_mul_inner(v, w) - p1 * p2) /
    (sqrtf(1 - p1 * p1) * sqrtf(1 - p2 * p2));
  d = d > 1.0 ? 1.0 : (d < -1.0 ? -1.0 : d);
  float angle = vec3_mul_inner(a2, axis) > 0 ? acos(d) : -acos(d);
  printf("p1: %.02f, p2: %.02f, d: %.02f, angle: %.02f\n", p1, p2, d, angle);

  quat_rotate(q, angle, axis);
  printf("q: (%.02f, %.02f, %.02f, %.02f)\n", q[0], q[1], q[2], q[3]);
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
  rot_by_axis_and_vertices(q2, w1, v2_, w2);

  quat_mul(q, q2, q1);
}

void quat_slerp_id(quat r, quat q, float t)
{
  float d = q[3];
  float sign = 1;
  if (d < 0) {
    d = -d;
    sign = -1;
  }

  if (d > 1 - SLERP_EPS) {
    /* use linear interpolation */
    quat_scale(r, q, sign * t);
    r[3] += 1 - t;
    quat_norm(r, r);
    return;
  }

  float s = sqrtf(1 - d * d);
  float theta = atan2(s, d);
  float a = sin(theta * (1 - t)) / s;
  float b = sin(theta * t) / s;

  quat_scale(r, q, sign * b);
  r[3] += a;
  return;
}
