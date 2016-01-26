#!/bin/bash

# static buf
./a.out 1 1000

# echo $RANDOM
# echo $RANDOM
# echo $RANDOM
# echo $RANDOM
# echo $RANDOM
# echo $RANDOM
# echo $(( RANDOM % 100))
# echo $(( RANDOM % 100))
# echo $(( RANDOM % 100))
# echo $(( RANDOM % 100))
# echo $(( RANDOM % 100))
# echo $(( RANDOM % 100))
# echo $(( RANDOM % 100))

# ./a.out 1 $(( RANDOM % 10000 ))
# $1 must be in {1,2,3,4}
for i in {1..100}
do
    myrand=$(( RANDOM % 10000 ))
    # echo -n $myrand " "
    # ./a.out $1 $myrand >/dev/null 2>&1
    valgrind --xml=yes --xml-file=out.xml ./a.out $1 $myrand >/dev/null 2>&1
    if [[ $? != 0 ]]; then
        echo "fail: " $myrand
    else
        echo "success: " $myrand
    fi
    ./valgrind-parser out.xml 85
    if [[ $? == 0 ]]; then
        echo "valgrind correct failure: " $myrand
    fi
done
