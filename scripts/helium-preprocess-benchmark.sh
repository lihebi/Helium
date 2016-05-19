#!/bin/bash

# preprocess benchmark
# 1. create cpped folder
# 2. copy all c and h files
# 3. preprocess all c and h files
# 4. remove line markers and included code from file other than the original source file

if [ $# != 1 ]; then
    echo "Preprocess benchmark."
    echo "Will delete cpped folder and output there."
    echo ""
    echo "Usage: this-script <path/to/benchmark/src/>"
    echo ""
    echo "Note: will copy all the .c and .h file from the top level of the path"
    exit 1
fi

rm -rf cpped
mkdir cpped
cp -r $1/*.c cpped/
cp -r $1/*.h cpped/

cd cpped
for fname in *.c; do
    echo $fname
    cc -E $fname > $fname.new
    mv $fname.new $fname
    helium-process-cpped.py $fname > $fname.new
    mv $fname.new $fname
done

for fname in *.h; do
    echo $fname
    cc -E $fname > $fname.new
    mv $fname.new $fname
    helium-process-cpped.py $fname > $fname.new
    mv $fname.new $fname
done
