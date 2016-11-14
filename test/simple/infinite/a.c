#include <stdio.h>

void foo(int a, int b) {
  for (int i=0;i<a;i++) {
    if (i==b) {
      i--;
    }
  }
}

int entry_1;
int entry_2;
int main(int argc, char *argv[]) {
  foo(entry_1, entry_2);
}

