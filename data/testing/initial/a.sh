#!/bin/bash

./parse-testing.py gzip-random.txt > gzip-random.csv
./parse-testing.py gzip-pairwise.txt > gzip-pairwise.csv
./parse-testing.py ncompress-random.txt > ncompress-random.csv
./parse-testing.py ncompress-pairwise.txt > ncompress-pairwise.csv
./parse-testing.py polymorph-random.txt > polymorph-random.csv
./parse-testing.py polymorph-pairwise.txt > polymorph-pairwise.csv
