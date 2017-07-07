struct A2;
struct A1 {
};
struct A2 {
  struct A1 a;
};


typedef struct A2 A2;
typedef struct A3 {
  struct A2 a2;
} A3Alias;


enum AEnum {
  AE_R,
  AE_G,
  AE_B
};

union AUnion {
  int b;
  char c;
};
