#!/bin/bash
# Input=$1
# Output=$2
# NThreads=$3
# NBoquetes=$4

for test in $1/* do
    ./tecnicofs-nosync $test "$test-1.txt" 1 1

    for thread in $(seq 2 $3) do
        ./tecnicofs-mutex $test "$test-$thread.txt" $thread $4
    done
done