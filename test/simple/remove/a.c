#include <stdio.h>
#include <assert.h>


// this file is to benchmark the removing debugging

int main() {
  int a=0;
  int b=1;
  int c=b-20;
  while (c<b) {
    a=c;
    c++;
  }
  a++;
  assert(a<b);
}
