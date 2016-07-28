#!/bin/bash

for bench in $1/*; do
    cd $bench
    echo "processing " $bench " .."
    rm -f helium_log.txt
    helium -s snippets/ cpped/ --print='ci,ce,col' & # --conf="run-test=false"
    cd ..
done
