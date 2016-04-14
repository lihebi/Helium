#!/bin/bash


# divide a bunch of files

if [ $# == 0 ]; then
    echo "Divide a bunch of slice files."
    echo ""
    echo "Usage: <this-script> /path/to/slice/file/folder"
    echo ""
    echo "Side Effect: ./tmp/ directory will be overwrote"
    echo "Output: the folder inside slice-bundle/ with extension removed"
    exit 1;
fi

mkdir slice-bundle

for file in $1/*.txt; do
    if [ -f $file ]; then
        dir=${file##*/} # remove traling directories
        dir=${dir%\.*} # remove extension
        dir=slice-bundle/$dir
        echo $file , output to $dir
        rm -rf $dir # I feel this is dangeous
        ./divide-slice.sh $file
        mv tmp $dir
    fi
done
