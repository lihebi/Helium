#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *helium_heap_addr[BUFSIZ];
int helium_heap_size[BUFSIZ];
int helium_heap_top = 0;
// the size of the found address. Should reset everytime before checking.
int helium_heap_target_size=-1;

void input() {
  char var;
  // HELIUM_INPUT_BEGIN
  scanf("%c", &var);
  // HELIUM_INPUT_END
}

void output() {
  char var = 'a';
  // HELIUM_OUTPUT_BEGIN
  printf("char_var = %c\n", var);
  fflush(stdout);
  // HELIUM_OUTPUT_END
}
