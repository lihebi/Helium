int func() {
  char *src;
  // @HeliumSegmentBegin
  char dst[100];
  strcpy(dst, src);
  // @HeliumSegmentEnd
}
