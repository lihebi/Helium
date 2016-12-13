#include <stdio.h>
#include <assert.h>


void foo(int a, int b) {
  a=b+1;
  assert(a>b);
}

void bar(int a, int b) {
  a = b-1;
  assert(a>b);
}

int main(int argc, char *argv[]) {
  
}
