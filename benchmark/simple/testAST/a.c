#include <stdio.h>
int foo(int a, struct AA *bb) {
  if (x>0) {
    while (x<10) {
      a=b;
      c=d;
      if (a>c) {
        sum+=c;
      } else if (a==c) {
        sum += con1;
      } else {
        sum += a;
        switch (a) {
        case 1: a++;a--;break;
        case 2:
        case 3: {
          a++;
          a--;
          break;
        }
        default: return 0;
        }
      }
    }
  } else {
    sum = 0;
    for (int i=0;i<8;i++) {
      sum += i;
    }
  }
}
