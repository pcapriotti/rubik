#ifndef _UTILS_H_
#define _UTILS_H_

void die(const char *msg);

enum {
  CATEGORY_ALPHANUM,
  CATEGORY_SPACE,
  CATEGORY_OTHER
};

int category(char c);
void skip_category(const char *s, int len, int cat, int direction, int *x);

#endif /* _UTILS_H_ */
