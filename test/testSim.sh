#! /bin/bash
# $1 is the in file, $2 is the out file, $3 is the args
./sim $3 < $1 > $2;
echo $3
