#!/usr/bin/env python3

print("project name,exit status,compile success count,compile error count,build rate")
with open("./buildrate-6:30pm1015.txt") as f:
    for line in f:
        if line.startswith('./'):
            print(line.strip(), end=',')
        if line.startswith('success'):
            print('0', end=',')
        if line.startswith('timeout'):
            print('-1', end=',')
        if line.startswith('SUCCESS'):
            # SUCCESS: 9 ERROR: 47 RATE: .16
            l = line.split()
            if len(l) == 5: l.append('0')
            print(l[1]+','+l[3]+','+l[5])
