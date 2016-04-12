#!/bin/bash

# do mlslice on the target folder

if [ $# == 0 ]; then
    echo "Usage: mlslice.sh /path/to/benchmark"
    echo "Will use the tmp/*.txt files as slice"
    exit 1;
fi


for file in tmp/*; do
    echo $file "========" 1>&2
    mlslice --slice=$file --benchmark=$1
done
