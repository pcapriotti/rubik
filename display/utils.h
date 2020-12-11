#ifndef DISPLAY_UTILS_H_
#define DISPLAY_UTILS_H_

#include <memory.h>
#include "linmath.h"

void die(const char *msg);

enum {
  CATEGORY_ALPHANUM,
  CATEGORY_SPACE,
  CATEGORY_OTHER
};

int category(char c);
void skip_category(const char *s, int len, int cat, int direction, int *x);

void rot_by_vertices(quat q, vec3 v1, vec3 v2, vec3 w1, vec3 w2);

void quat_slerp_id(quat r, quat q, float t);

#endif /* DISPLAY_UTILS_H_ */
