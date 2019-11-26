#!/bin/bash
# Input=$1
# Output=$2
# NThreads=$3
# NBoquetes=$4

# nameRight = test1
# test = inputs/test1.txt


mkdir -p $2     # criacao de pasta para os outputs

for test in $1/* 
do

    nameRight=${test#"$1/"}             # retira o prefixo inputs/
    nameRight=${nameRight%".txt"}       # retira o sufixo .txt
    
    echo InputFile=​ $test ​ NumThreads=​ 1
    ./tecnicofs-nosync $test "$2/$nameRight-1.txt" 1 1 | tail -1


    for thread in $(seq 2 $3) 
    do
        echo InputFile=​ $test ​ NumThreads=​ $thread
        ./tecnicofs-mutex $test "$2/$nameRight-$thread.txt" $thread $4 | tail -1

    done
done