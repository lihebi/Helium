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

rm -rf helium
mkdir helium

# 0. gather all benchmarks and their meta data
# 0.1 uncompress benchmarks

# 1. put all benchmarks into a folder
#    this script runs against that folder
for bench in $1/*; do
    # 2. for each benchmark, create a dummy project: XXX-cpped/cpped/
    echo "== processing " $bench " ..."
    # this bench is the relative path to .tar.gz
    if [[ -d $bench ]]; then
        simple_name=${bench##*/}
        cp -r $bench $simple_name
    elif [[ $bench == *.tar.gz ]]; then
        echo "uncompressing .."
        tar zxvf $bench > /dev/null 2>&1
        bench=${bench%.tar.gz}
    elif [[ $bench == *.zip ]]; then
        echo "uncompressing .."
        unzip $bench > /dev/null 2>&1
        bench=${bench%.zip}
    elif [[ $bench == *.tgz ]]; then
        echo "uncompressing .."
        tar xvf $bench > /dev/null 2>&1
        bench=${bench%.tgz}
    else
        continue
    fi
    bench=${bench##*/}
    # this is simply the name itself
    mkdir helium/$bench
    mkdir helium/$bench/orig
    mkdir helium/$bench/src
    mkdir helium/$bench/tmp
    mkdir helium/$bench/cpped
    mv $bench helium/$bench/orig
    # 3. find all .c and .h files into that folder
    #    FIXME do not consider same name for now
    #    FIXME might have many main function ...
    find "helium/$bench/orig" -name "*.[c|h]" -exec cp "{}" helium/$bench/src/ \;
    cd helium/$bench
    echo "== Compiler Preprocessing .."
    for file in src/*; do
        # 4. use cc -E -nostdinc to process, rename back to the same name
        simple_name=${file##*/}
        # echo $simple_name
        cd src
        # this will determine what the line marker looks like
        # FIXME NOW The include files are buggy if not found!
        # so we'd better process it from original location
        # cpp will terminate for unfound local header files
        # Maybe just leave it, because the error I found for apache is just the file is missing, not loation problem
        # $CC -E -nostdinc "$simple_name" > "../tmp/$simple_name" 2>/dev/null
        $CC -E "$simple_name" > "../tmp/$simple_name" 2>/dev/null
        cd ../tmp
        # 5. use helium-process-cpped.py to remove extra line marker staffs, rename back
        helium-process-cpped.py "$simple_name" > "../cpped/$simple_name"
        cd ..
    done
    # 6. create snippetdb, the name should be renamed to XXX-cpped/snippet/
    # helium --create-snippet-db helium/$bench/src
    # mv snippets helium/$bench/
    echo "== Creating snippet db ..."
    time helium --create-snippet-db cpped # > /dev/null
    cd ../..
done    

# build rate & search efficiency
# 1. define a txt file for POI selection!
# 2. do not include test, run context search, for each context, record build rate.
#    Limit the procedure to 4
# 3. also, list the search times

## Try to find problems! Otherwise others would assume everything works fine, and expect results

# dynamic debugging
# template implementation
# more general transfer,invariant,pre-condition stop point
