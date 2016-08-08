#!/bin/bash

# simply invoke this in a benchmark directory

benchmark=`pwd`
benchmark=${benchmark##*/}
# command=""
# echo $command
# bash -c $command

helium -s snippets -c src\
       --print='ci,ce,col'\
       --whole-poi=/tmp/poi/poi.org -b $benchmark\
       cpped\
       # --conf="instrument-strlen=true"\ # for buffer overflow, bugbench
       --conf="test-number=1"\ # for build rate testing
       $*
