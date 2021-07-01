#! /bin/bash
ulimit -s 65536
set -x
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ind generator_att_ind.c
./generator_att_ind $1 $2 $3
gcc -O0 -Os -c FOO_[0-9]*.c
gcc -O0 -Os -c FOO_main.c
gcc -O0 -Os FOO_main.o FOO_0*.o -o FOO_static
