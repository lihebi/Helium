#!/bin/bash

# checkout all prjects in folder

cd $1
if [ $? != 0 ]; then exit 1; fi
for fname in *
do
  echo checking out: $fname
  cd $fname && git checkout -- . && cd ..
  if [ $? == 0 ]; then
    echo "OK"
  else
    echo "FAIL"
  fi
done
