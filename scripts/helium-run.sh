#!/bin/bash

for bench in $1/*; do
    echo "processing " $bench " .."
    simple_name=${bench##*/}
    poi_file=/tmp/poi/$simple_name.poi.txt
    if ! [ -f $poi_file ]; then
        echo "NO POI File: $poi_file"
        continue
    fi

    # processing
    cd $bench
    rm -f helium_log.txt helium_dump.txt
    helium -s snippets/ cpped/ --print='ci,ce,col' -c src --poi=$poi_file
    if [ $? == 0 ]; then
        echo $bench " terminate successfully"
    fi
    cd ..
done
