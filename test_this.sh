#!/bin/bash

if [ -f foo ]
then
    echo this script requies use of a file foo
    exit 1
fi

if [ -f bar ]
then
    echo this script requies use of a file bar
    exit 1
fi

for X in *
do
    if [ -f "$X" ]
    then
        printf "checking %s\n" $X
        filesize=$(stat -c '%s' $X)
        printf "uncompressed size:\t%d\n" $filesize
        ./sample -c -i $X -o foo
        ./sample -d -i foo -o bar
        diff $X bar
        filesize=$(stat -c '%s' foo)
        printf "traditional size:\t%d\n" $filesize
        ./sample -C -c -i $X -o foo
        ./sample -C -d -i foo -o bar
        diff $X bar
        filesize=$(stat -c '%s' foo)
        printf "canonical size:\t\t%d\n\n" $filesize
        rm foo
        rm bar
    fi
done

exit 0
