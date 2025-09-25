#! /bin/sh
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

taskset 0x4 ./FOO_static3_$1_$2_$3 $4
