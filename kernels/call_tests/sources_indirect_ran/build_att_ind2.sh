#! /bin/bash
ulimit -s 65536
set -x
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ind2 generator_att_ind2.c
./generator_att_ind2 $1 $2 $3
make -f Makefile2 -j 40
