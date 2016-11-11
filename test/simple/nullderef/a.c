#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct _demo {
  int id;
  char *f;
} demo;

bool b;
bool c;

demo somedemo;

void bar(demo *z, demo *x) {
  demo *p = z->f;
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

int main(int argc, char* argv[]) {
  demo *z;
  demo *x;
  x=(demo*)malloc(sizeof(demo));
  z=(demo*)malloc(sizeof(demo));
  x->f=argv[1];
  z->f=argv[2];
  bar(z, x);
}
