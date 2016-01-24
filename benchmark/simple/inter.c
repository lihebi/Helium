int foo(char *from) {
  int i, sum = 0;
  if (strlen(from) > 9) exit(0);
  for (i=0;i<100;i++) {
    sum += i;
  }
  bar(from);
}

int bar(char *from) {
  char to[10];
  int a;
  int b;
  int sum=0;
  a = 8;
  b  = ++a;
  for (sum=0;sum<100;) {
    sum += a+b;
  }
  sum = a+b;
  // @HeliumStmt
  strcpy(to, from);
}
