#! /bin/bash
# $1 is the in file, $2 is the out file, $3 is the args
{ time $(../alloc $1 $2 $3); } 2> results_time_raw;
grep "real" results_time_raw > results_time;
