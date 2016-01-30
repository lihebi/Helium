int func() {
  char *src;
  // @HeliumSegmentBegin
  static char s_buf[100];
  strcpy(s_buf, src); // @HeliumLineMark
  // @HeliumSegmentEnd
}
