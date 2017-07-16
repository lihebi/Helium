#include <stdio.h>
#include <string.h>


int func(char* from) {
  char buf[20];
  // @HeliumStmt
  strcpy(buf, from);
}
