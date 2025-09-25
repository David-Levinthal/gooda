#! /bin/bash
ulimit -s 65536
set -x
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ind16 generator_att_ind16.c
./generator_att_ind16 $1 $2 $3
gcc -O0 -Os -c FOO_[0-9]*.c
gcc -O0 -Os -c FOO_main.c
gcc -O0 -Os FOO_main.o FOO_0*.o -o FOO_static16_$1_$2_$3
rm FOO_0*
rm FOO_main* FOO.h
