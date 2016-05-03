#!/bin/bash

if [ $# == 0 ]; then
    echo "Remove absolute, leaving only relative path."
    echo "this should be benchmark specific!"
    echo "Currently: gzip-1.7"
    echo ""
    echo "Usage: "
    echo "<this-script> /path/to/slice/with/abs-path"
    exit 1
fi

cat $1 | sed -e 's/\/home\/hebi\/benchmark\/gzip-1.7\///g'
