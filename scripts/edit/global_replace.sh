#!/bin/bash

# globally replace a string with a pattern.

prefix='Config::Instance()->GetString'
src=$prefix'("\(.*\)_\(.*\)")'
dst=$prefix"(\1-\2)"

# find . -name *.cc -exec sed -ne "s/$src/$dst/p" {} \;
find . -name *.cc -exec sed -ie "s/$src/$dst/g" {} \;
