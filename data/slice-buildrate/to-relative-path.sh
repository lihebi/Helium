#!/bin/bash

# sed -i -e 's/\/home\/hebi\/benchmark\/GithubSmallCProjects\///g' $1

if [ $# == 0 ]; then
    echo "convert to relative path."
    echo "Usage: <this-script> /path/to/slice/files"
    exit 1;
fi

for file in $1/*; do
    if [ -f $file ]; then
        # get the benchmark name, without any path and suffix
        benchmark=${file%.*}
        benchmark=${benchmark##*/}
        # get the prefix to remove
        line=`cat $file | grep "slice criteria" | head -1`
        line=${line#slice criteria: }
        line=${line%$benchmark*}$benchmark/
        line=${line//\//\\\/} # replace / to \/, so that it can be used in sed
        # remove the prefix
        sed -i "" -e "s/$line//g" $file
    fi
done
