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


global_suc_min=`cat global.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | head -1`
global_suc_max=`cat global.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | tail -1`
global_suc_count=`cat global.valgrind.txt | grep success | sed "s/success:  //g" | wc -l`

global_fail_min=`cat global.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | head -1`
global_fail_max=`cat global.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | tail -1`
global_fail_count=`cat global.valgrind.txt | grep fail: | sed "s/fail:  //g" | wc -l`

global_valgrind_min=`cat global.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | head -1`
global_valgrind_max=`cat global.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | tail -1`
global_valgrind_count=`cat global.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | wc -l`

# all time 28s, 100 tests
## static
static_suc_min=`cat static.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | head -1`
static_suc_max=`cat static.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | tail -1`
static_suc_count=`cat static.valgrind.txt | grep success | sed "s/success:  //g" | wc -l`

static_fail_min=`cat static.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | head -1`
static_fail_max=`cat static.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | tail -1`
static_fail_count=`cat static.valgrind.txt | grep fail: | sed "s/fail:  //g" | wc -l`

static_valgrind_min=`cat static.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | head -1`
static_valgrind_max=`cat static.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | tail -1`
static_valgrind_count=`cat static.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | wc -l`
##stack
stack_suc_min=`cat stack.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | head -1`
stack_suc_max=`cat stack.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | tail -1`
stack_suc_count=`cat stack.valgrind.txt | grep success | sed "s/success:  //g" | wc -l`

stack_fail_min=`cat stack.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | head -1`
stack_fail_max=`cat stack.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | tail -1`
stack_fail_count=`cat stack.valgrind.txt | grep fail: | sed "s/fail:  //g" | wc -l`

stack_valgrind_min=`cat stack.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | head -1`
stack_valgrind_max=`cat stack.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | tail -1`
stack_valgrind_count=`cat stack.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | wc -l`
## heap

heap_suc_min=`cat heap.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | head -1`
heap_suc_max=`cat heap.valgrind.txt | grep success | sed "s/success:  //g" | sort -n | tail -1`
heap_suc_count=`cat heap.valgrind.txt | grep success | sed "s/success:  //g" | wc -l`

heap_fail_min=`cat heap.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | head -1`
heap_fail_max=`cat heap.valgrind.txt | grep fail: | sed "s/fail:  //g" | sort -n | tail -1`
heap_fail_count=`cat heap.valgrind.txt | grep fail: | sed "s/fail:  //g" | wc -l`

heap_valgrind_min=`cat heap.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | head -1`
heap_valgrind_max=`cat heap.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | sort -n | tail -1`
heap_valgrind_count=`cat heap.valgrind.txt | grep valgrind | sed "s/valgrind correct failure:  //g" | wc -l`


echo type, suc min, suc max, suc count, fail min, fail max, fail count, valgrind min, valgrind max, valgrind count, time
echo static, $static_suc_min,$static_suc_max,$static_suc_count,$static_fail_min,$static_fail_max,$static_fail_count, $static_valgrind_min, $static_valgrind_max, $static_valgrind_count, 28
echo stack, $stack_suc_min,$stack_suc_max,$stack_suc_count,$stack_fail_min,$stack_fail_max,$stack_fail_count, $stack_valgrind_min, $stack_valgrind_max, $stack_valgrind_count, 28
echo heap, $heap_suc_min,$heap_suc_max,$heap_suc_count,$heap_fail_min,$heap_fail_max,$heap_fail_count, $heap_valgrind_min, $heap_valgrind_max, $heap_valgrind_count, 28
echo global, $global_suc_min,$global_suc_max,$global_suc_count,$global_fail_min,$global_fail_max,$global_fail_count, $global_valgrind_min, $global_valgrind_max, $global_valgrind_count, 28


