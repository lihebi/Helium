#!/bin/bash


# inputs are in the "input/" directory
# the files are named by: 0.txt, 1.txt, ...
# the output files will be in "output/" folder (the folder must be exist). The file name will be the same as input
# both the stdout and stderr will be redirect to the output file


echo "Running all tests .."

allpids=
for inputfile in input/*; do
    file=${inputfile##*/}
    outputfile="output/$file"
    # echo "Input file: $inputfile"
    # echo "Output file: $outputfile"
    timeout -k 1 1 ./a.out <$inputfile >$outputfile 2>/dev/null &
    pid=$!
    # echo "PID: $pid"
    allpids="$allpids $pid"
done

# echo "All PIDs: $allpids"

# echo "Sleeping 2 sec .."
# run them for 2 seconds

# timeout 2 wait

wait $allpids

# if [ $? == 124 ]; then
#     echo "Timed out."
# fi

echo "Collecting return status .."

res=
# collect data
for pid in $allpids; do
    # test if they finishes
    # kill -s 0 $pid 2>/dev/null
    # get the return code by PID
    wait $pid
    res="$res $?"
    # echo "Return status: $res"
done

# store the return code with the tests
echo $res > test-return-codes.txt
echo "Wrote return code to test-return-codes.txt"


# concate all the test results into one result.txt
# this is to be compatible with eixistins analysis code
# may change in the future




echo "Converting output into result.txt .."
input=
output=
for file in input/*; do
    input+=" $file"
done
for file in output/*; do
    output+=" $file"
done


read -a inputarr <<< $input
read -a outputarr <<< $output
read -a resarr <<< $res

# ASSERT equal size?
for ((i=0;i<${#inputarr[@]};i++)); do
    code=${resarr[i]}
    cat ${outputarr[i]} >> result.txt
    if [ $code == "0" ]; then
        echo "HELIUM_TEST_SUCCESS" >> result.txt
    else
        echo "HELIUM_TEST_FAILURE" >> result.txt
    fi
done

echo "Converting to csv file .."
helium-output-to-csv.py result.txt > result.csv
