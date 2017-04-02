#include <vector>

std::vector<const char *> programs = {
R"prefix(
int foo() {
  if (b>0) {
    d=c+e;
  }
}
)prefix",
R"prefix(
int foo() {
  if (b>0) {
    d=c+e;
  } else if (b < 10) {
    a=8;
  }
}
)prefix",
R"prefix(
int foo() {
  for (int i=0;i<c;i++) {
    a+=i;
  }
}
)prefix",
R"prefix(
int foo() {
  while (a<c) {
    a=b;
    b=c;
  }
}
)prefix",
R"prefix(
int foo(int a, int b) {
  int c=8,d;
}
)prefix",
R"prefix(
int foo(int a, int b) {
  int c=8;
  while (a<c) {
    a=b;
    if (b>0) {
      d=c+e;
    }
    b=c;
  }
}
)prefix",
R"prefix(
int foo() {
  int a=8,b=9;
  switch (a) {
  case 1: a=9; b=10; break;
  case 2: {a=b+1;}
  default: break;
  }
}
)prefix"
};
