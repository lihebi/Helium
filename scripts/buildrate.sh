#!/bin/bash

for fname in $1/*
do
  echo $fname
  # helium --pre $fname > /dev/null 2>&1
  if [ `uname` == "Darwin" ]; then
    gtimeout 60 helium $fname > /dev/null 2>&1
  else
    timeout 60 helium $fname > /dev/null 2>&1
  fi
  if [ $? == 0 ]; then
    echo "success";
  else
    echo "timeout";
  fi
  success_count=`cat /tmp/helium/log/rate.txt | grep success | wc -l`
  error_count=`cat /tmp/helium/log/rate.txt | grep error | wc -l`
  buildrate=`echo "scale=2; $success_count/($success_count+$error_count)$" | bc`
  echo SUCCESS: $success_count ERROR: $error_count RATE: $buildrate
done
