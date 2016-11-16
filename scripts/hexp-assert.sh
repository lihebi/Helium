#!/bin/bash

# assertion test script

# $1: /path/to/folder/containing/benchmarks
# $2: /path/to/headered/pois
# Output: helium-result.txt

# Also, use tee to run the script!

# 2>&1 | tee -ai log.txt



for name in $1/*; do
    echo "============ " ${name##*/} | tee -a helium-result.txt
    timeout 10m helium --segment-per-poi-limit=100 --valid-poi-limit=100 $name --poi-file=/home/hebi/github/benchmark/github/assert/assert60.csv 2>&1 | tee -a helium-assert-result.txt
    [ $? == 143 ] && echo "====== Benchmark $name Timeout" | tee -a helium-result.txt
    rm -rf /tmp/helium*
done
