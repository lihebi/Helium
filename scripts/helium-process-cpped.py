#!/usr/bin/env python3

"""
# 1. remove everything between # xxx 1 and # gzip.c 2

Usage:
thisscript gzip.cpp.c
"""

import sys
import re

assert len(sys.argv) > 1
filename = sys.argv[1]

output=False
count=0
for line in open(filename):
    # print(line)
    # line = "# 3 \"fdsl\""
    # line = "# 1 \"builtin\" 3"
    # line = "# 1 \"gzip.c\" 2"
    pattern = ("^#\s*\d+\s+"
               "\"([\w\.-<>]+)\"" # first capturing group, the file name
               "\s+(\d+)"
               )
    m = re.search(pattern, line)
    if output:
        if not line.startswith('#') and len(line) != 1:
            print(line, end='')
    if m:
        count += 1
        f = m.group(1) # file name in line marker in the pre-processed c file
        flag = m.group(2) # the flag associated with above line
        # print("0: " + g0)
        # print("1: " + g1)
        if flag == '2' and f == filename:
            output=True
        if (flag == '1'):
            output = False
