#! /bin/sh
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH

taskset 0x4 ./FOO_static_1000 10000
