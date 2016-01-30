#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main() {
  int size;
  scanf("%d", &size);
  char buf[size];
  int heap_size;// = 8;
  scanf("%d", &heap_size);
  char *src = (char*)malloc(sizeof(char)*heap_size);
  scanf("%s", src);
  strcpy(buf, src);
  return 0;
}
