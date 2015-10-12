#!/bin/bash

for fname in $1/*
do
  echo $fname
  # helium --pre $fname
  # helium $fname
  success_count=`cat /tmp/helium_buildrate.txt | grep success | wc -l`
  all_count=`cat /tmp/helium_buildrate.txt | wc -l`
  buildrate=`echo "scale=2; $success_count/$all_count" | bc`
  echo $success_count/$all_count= $buildrate
done
