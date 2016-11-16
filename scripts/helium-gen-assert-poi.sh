#!/bin/bash


bench=${1##*/}
if [ -z $bench ]; then
    bench=${1%/}
    bench=${bench##*/}
fi
if [ -z $bench ]; then
    >&2 echo "Cannot get benchmark name from $1."
    exit 1;
fi

for file in $1/src/*.c; do
    >&2 echo $file
    helium-gen-assert-poi.awk $file | awk -v bench="$bench" -v file=${file##*/} '{print bench "," file "," $0}'
    # cmd="helium-gen-assert-poi.awk $file | awk '{print \"${bench},${file##*/},\" \$0}'"
    # cmd1="helium-gen-assert-poi.awk $file"
    # cmd2="awk '{print \"${bench},${file##*/},\" \$0}'"
    # echo $cmd
    # echo $cmd
    # $cmd
    # $cmd1 | $cmd2
    # "${bench},${file##*/},$line,stmt"
done
