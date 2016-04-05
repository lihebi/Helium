#include <stdio.h>
struct AA {
  int a;
};

int foo(int a, struct AA *bb) {
  int b=0;
  int c,d=1;
  int x=3;
  int sum=8;
  int con1=bb->a;
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
  return sum;
}

int main() {
  struct AA a;
  a.a = 8;
  foo(1, &a);
}
