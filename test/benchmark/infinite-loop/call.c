#include <stdio.h>

/*******************************
 ** Recursive calls
 *******************************/

void foo();
void bar();


void foo() {
  bar();
}

void bar() {
  foo();
}
