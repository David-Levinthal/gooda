#! /bin/bash
ulimit -s 65536
set -x
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ind3 generator_att_ind3.c
./generator_att_ind3 $1 $2 $3
make -f Makefile3 -j 40
