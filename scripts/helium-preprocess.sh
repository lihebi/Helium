#!/bin/bash

CC=${CC=clang}

type $CC >/dev/null 2>&1 || { echo >&2 "I require $CC but it's not installed.  Aborting."; exit 1; }

# create snippet db for all the benchmarks in argument

if [[ $# -eq 0 || $1 == '-h' ]]; then
    echo "create snippet db for all the benchmarks in argument";
    echo 'Usage: ';
    echo 'this.sh /path/to/folder/contains/benchmarks'
    exit 0;
fi

# $1: name of archive
# $2: name of destination folder
function uncompress() {
    if [[ -d $1 ]]; then
        cp -r $1 $2
    elif [[ $1 == *.tar.gz ]]; then
        tar zxvf $1 -C $2 > /dev/null 2>&1
    elif [[ $1 == *.zip ]]; then
        unzip $1 -d $2 > /dev/null 2>&1
    elif [[ $1 == *.tgz ]]; then
        tar xvf $1 -C $2 > /dev/null 2>&1
    else
        continue
    fi
}

output_folder="helium_preprocessed"
rm -rf $output_folder
bench=$1
mkdir -p $output_folder
mkdir $output_folder/orig
mkdir $output_folder/src
mkdir $output_folder/tmp
mkdir $output_folder/cpped

# copy or uncompress the code into "orig" folder
uncompress $bench $output_folder/orig

# copy all .c and .h files into "src" folder
# FIXME do not consider same name for now
find "$output_folder/orig" -name "*.[c|h]" -exec cp "{}" $output_folder/src/ \;


cd $output_folder
echo "== Compiler Preprocessing .."
for file in src/*; do
    # 4. use cc -E -nostdinc to process, rename back to the same name
    simple_name=${file##*/}
    # inside src folder, preprocess all files and put the result into "tmp" folder
    cd src
    $CC -E -include $HELIUM_HOME/lib/config/config.h "$simple_name" > "../tmp/$simple_name" 2>/dev/null
    cd ../tmp
    # 5. use helium-process-cpped.py to remove extra line marker staffs, rename back
    # in "tmp" folder, call the preprocess script, and put the result into "cpped" folder, as final result
    helium-process-cpped.py "$simple_name" > "../cpped/$simple_name"
    cd ..
done

# 6. create snippetdb, the name should be renamed to XXX-cpped/snippet/
# helium --create-snippet-db helium/$bench/src
# mv snippets helium/$bench/
echo "== Creating snippet db ..."

if [ -n $2 ]; then
    echo "== using timeout $timeout ..."
    # FIXME the time output will be redirected
    timeout $2 time helium --create-snippet-db cpped > /dev/null 2>&1
    if [ $? == 124 ]; then
        echo "== EE: Timed out"
    fi
else
    time helium --create-snippet-db cpped > /dev/null 2>&1
fi
