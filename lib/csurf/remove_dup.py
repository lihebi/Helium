#!/usr/bin/env python
import sys

f = open(sys.argv[1])
s = set()
for line in f:
  if line not in s:
    s.add(line)
    print line,
