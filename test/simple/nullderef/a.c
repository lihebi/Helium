#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct _demo {
  int id;
  struct _demo *f;
} demo;

bool b;
bool c;

demo somedemo;

void bar(demo *z, demo *x) {
  char *p = z->f;
  demo *y;
  int d;
  if (c) {
    d = 0;
  } else {
    d = 1;
  }
  if (b) {
    y = z;
  } else {
    y = x->f;
  }
  *y = somedemo;
}

demo *entry_1;
demo *z;
demo *x;

int main() {
  x->f=entry_1;
  bar(z, x);
}
