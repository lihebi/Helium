#!/bin/bash

for fname in *.txt; do
    parse-testing.py $fname > $fname.csv
done
