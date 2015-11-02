
struct B {
  int a;
};

struct A {
  int a;
  struct B b;
  struct A* next;
};
