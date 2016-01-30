int func(char *from) {
  int bound1;
  int bound2;
  // garbage code
  int sum = 0;
  for (int i=0;i<100;i++) {
    sum += i;
  }
  // setting bound
  bound2 = 200;
  bound1 = 100;
  char buf[100];
  // incorrect bounds checking
  if (strlen(from) < bound2) {
    // @HeliumSegmentBegin
    strcpy(buf, from);
    // @HeliumSegmentEnd
  }
}
