#include <stdio.h>

void foo(int x, int y, int z) {
  x+y+z;
}

int main() {
  int x = 1;
  int y = x+1;
  int z = x+y;
  if (x==1) {
    foo(x,y,z);
  }
}
