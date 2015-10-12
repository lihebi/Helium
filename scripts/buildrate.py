#!/usr/bin/env python3
import os
import sys

for root,dirs,files in os.walk(sys.argv[1]):
    if root == sys.argv[1]:
        for d in dirs:
            print("preprocessing " + d)
            call("helium --pre "+d, shell=True, stdout=DEVNULL, stderr=DEVNULL)
            call("helium " + d, shell=True, stdout=DEVNULL, stderr=DEVNULL)
            call("cat")
