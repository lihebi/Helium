#include <stdio.h>

void foo() {
  int c;
  c=2;
  c=3;
}

void bar() {
  int a;
  a=1;
  foo();
  int b;
  a=1;
}
