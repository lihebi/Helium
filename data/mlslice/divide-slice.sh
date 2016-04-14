#!/bin/bash

# divide the slice file into several separate slice file, each contains exactly one slice criteria.


if [ $# == 0 ]; then
    echo "usage: divide-slice /path/to/slice-file.txt"
    echo ""
    echo "this will REMOVE ./tmp/ folder and populate the result files into the folder"
    echo "1. divide"
    echo "2. sort & uniq"
    exit 1;
fi

rm -rf tmp
mkdir tmp

# divide the file by =====, into tmp/<num>.txt
awk -f ./divide-slice.awk $1

# first line remove slice criteria, => "ralative/path/to/xxx.c:34"
# sort and then uniq the slices
for file in tmp/*; do
    head -1 $file | awk '{print $3}' > $file-new
    cat $file | awk '!/slice criteria/ {print;}' | sort | uniq >> $file-new
    mv $file-new $file
done



# awk 'BEGIN {filecount=0}
# {current_filename = "tmp/" filecount  ".txt"}
# {last_filename = "tmp/" filecount-1  ".txt"}
# /====/  {filecount++;close(last_filename)}
# /slice criteria/ {print $3 > current_filename}
# !/====/ && !/slice criteria/ {print  > current_filename}
# ' $1
