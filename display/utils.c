#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
