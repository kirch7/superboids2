#!/bin/bash

function setindex {
    safeindexdiff=10
    if [ -f $safeindexdiff".png" ]
    then
	index0=$(ls *.png | sort -n | tail -n 1)
	index0=$(echo $index0 | awk -F'.' '{print $1}')
	index0=$(( index0 - safeindexdiff ))
    else
	index0=0
    fi

    if [ $(du -b *.png | awk 'BEGIN {sum=0;} {if ($1 < 100) {sum++;}} END {print sum}') != 0 ]
    then
	index0=$(du -b *.png | awk '{if ($1 < 100) {print $2;}}' | awk -F '.' '{print $1}' | sort -n | head -n 1)
    fi
}

if [ -n "$1" ]
then
    dotdat=$1
else
    dotdat=$(ls *plainprint.dat | sort -n | tail -n 1)
fi

setindex

echo "Calling parallel-gnuplot"
./parallel-gnuplot -d $dotdat -g ./tangerine.gp --initial $index0 --stop 1

exit $?
