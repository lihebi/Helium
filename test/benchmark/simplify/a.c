void foo(int a) {
  if (a > 10) {
    if (a > 18 && a < 20) {
      // @HeliumStmt
      a++;
    }
  }
}
