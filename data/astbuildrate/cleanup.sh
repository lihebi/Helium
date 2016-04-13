#!/bin/bash

# clean up the build rate csv file

if [ $# == 0 ]; then
    echo "Usage: ./cleanup.sh /path/to/buildrate/result.csv"
    echo "The output will be out.csv"
    exit 1;
fi

# cat $1 | awk -f ./remove-dump-start-stop.awk

cat $1 | awk '!/====/ && !/benchmark/ && !/\.\// {print}' > tmp.csv
cat tmp.csv | head -1 > out.csv
cat tmp.csv | awk '!/leaf/ && !/^$/ {print}' >> out.csv
rm tmp.csv
echo "Outputed to out.csv"
