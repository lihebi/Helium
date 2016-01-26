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

for i in {1..1000}
do
    myrand=$((RANDOM % 10000 ))
    # echo -n $myrand " "
    ./a.out 3 $myrand >/dev/null 2>&1
    if [[ $? != 0 ]]; then
        echo "fail: " $myrand
    else
        echo "success: " $myrand
    fi
done
