#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buf[1024];

void foo(char *str) {
  strcpy(buf, str);
}

int main(int argc, char *argv[]) {
  foo(argv[0]);
}
