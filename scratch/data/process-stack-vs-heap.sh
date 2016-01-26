#!/bin/bash

# cat global.txt | grep success  | sed -e "s/success:  //g" > global-success.csv
# cat global.txt | grep fail  | sed -e "s/fail:  //g" > global-fail.csv

# cat heap.txt | grep success  | sed -e "s/success:  //g" > heap-success.csv
# cat heap.txt | grep fail  | sed -e "s/fail:  //g" > heap-fail.csv

# cat static.txt | grep success  | sed -e "s/success:  //g" > static-success.csv
# cat static.txt | grep fail  | sed -e "s/fail:  //g" > static-fail.csv

# cat stack.txt | grep success  | sed -e "s/success:  //g" > stack-success.csv
# cat stack.txt | grep fail  | sed -e "s/fail:  //g" > stack-fail.csv

global_suc_min=
global_suc_max=
global_suc_count=
global_fail_min=
global_fail_max=
global_fail_count=


global_suc_min=`cat global-success.csv | sort -n | head -1`
global_suc_max=`cat global-success.csv | sort -n | tail -1`
global_suc_count=`cat global-success.csv | wc -l`
global_fail_min=`cat global-fail.csv | sort -n | head -1`
global_fail_max=`cat global-fail.csv | sort -n | tail -1`
global_fail_count=`cat global-fail.csv | wc -l`

static_suc_min=`cat static-success.csv | sort -n | head -1`
static_suc_max=`cat static-success.csv | sort -n | tail -1`
static_suc_count=`cat static-success.csv | wc -l`
static_fail_min=`cat static-fail.csv | sort -n | head -1`
static_fail_max=`cat static-fail.csv | sort -n | tail -1`
static_fail_count=`cat static-fail.csv | wc -l`

stack_suc_min=`cat stack-success.csv | sort -n | head -1`
stack_suc_max=`cat stack-success.csv | sort -n | tail -1`
stack_suc_count=`cat stack-success.csv | wc -l`
stack_fail_min=`cat stack-fail.csv | sort -n | head -1`
stack_fail_max=`cat stack-fail.csv | sort -n | tail -1`
stack_fail_count=`cat stack-fail.csv | wc -l`

heap_suc_min=`cat heap-success.csv | sort -n | head -1`
heap_suc_max=`cat heap-success.csv | sort -n | tail -1`
heap_suc_count=`cat heap-success.csv | wc -l`
heap_fail_min=`cat heap-fail.csv | sort -n | head -1`
heap_fail_max=`cat heap-fail.csv | sort -n | tail -1`
heap_fail_count=`cat heap-fail.csv | wc -l`


echo type, suc min, suc max, suc count, fail min, fail max, fail count
echo global, $global_suc_min,$global_suc_max,$global_suc_count,$global_fail_min,$global_fail_max,$global_fail_count
echo static, $static_suc_min,$static_suc_max,$static_suc_count,$static_fail_min,$static_fail_max,$static_fail_count
echo stack, $stack_suc_min,$stack_suc_max,$stack_suc_count,$stack_fail_min,$stack_fail_max,$stack_fail_count
echo heap, $heap_suc_min,$heap_suc_max,$heap_suc_count,$heap_fail_min,$heap_fail_max,$heap_fail_count


