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
    pwd
    cd $bench
    pwd
    rm -f helium_log.txt helium_dump.txt
    # helium -s snippets/ cpped/ --print='ci,ce,col' -c src --poi=$poi_file
    ##############################
    ## CONFIG
    ##############################
    # when testing build rate, use test-number=1
    # do NOT use run-test=false, that is buggy
    # helium -s snippets/ cpped/ --print='ci,ce,col' --conf='instrument-strlen=true' -c src --whole-poi=/tmp/poi/poi.org -b $simple_name
    helium -s snippets/ cpped/ --print='ci,ce,col,t' --conf='test-number=1' -c src --whole-poi=/tmp/poi/poi.org -b $simple_name
    
    if [ $? == 0 ]; then
        echo $bench " terminate successfully"
    fi
    cd ..
done
