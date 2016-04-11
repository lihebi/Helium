#!/usr/bin/env python
import sys
import os

for root, dirs, files in os.walk(sys.argv[1]):
    for f in files:
        i=0
        if not f.endswith('.c'): continue
        for line in open(root+'/'+f):
            i+=1
            if "assert(" in line:
                print os.path.abspath(root+'/'+f) + ':' + str(i)
                break
