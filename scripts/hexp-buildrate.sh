#!/bin/bash

# build rate test script

# $1: /path/to/folder/containing/benchmarks
# $2: /path/to/headered/pois
# Output: helium-result.txt

# Also, use tee to run the script!

# 2>&1 | tee -ai log.txt



for name in $1/*; do
    echo "============ " ${name##*/} | tee -a helium-result.txt
    timeout 30m helium --run-test=false --instrument-io=false --segment-per-poi-limit=100 --valid-poi-limit=100 $name --poi-file=$2/${name##*/}.csv 2>&1 | tee -a helium-result.txt
    [ $? == 143 ] && echo "====== Benchmark $name Timeout" | tee -a helium-result.txt
    rm -rf /tmp/helium*
done
