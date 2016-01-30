int func() {
  char *src;
  // @HeliumSegmentBegin
  char *d_buf;
  d_buf = (char*)malloc(sizeof(char)*100);
  strcpy(d_buf, src); // @HeliumLineMark
  // @HeliumSegmentEnd
}
