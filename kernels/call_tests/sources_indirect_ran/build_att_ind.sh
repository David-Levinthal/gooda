#! /bin/bash
ulimit -s 65536
set -x
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ind generator_att_ind.c
./generator_att_ind $1 $2 $3
make -f Makefile1 -j 40
