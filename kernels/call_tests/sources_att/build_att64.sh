#! /bin/bash
set -x
ulimit -s 65536
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att64 generator_att64.c
./generator_att64 $1 $2 $3
make -f Makefile3 -j 8
#gcc -O0 -g -c FOO_[0-9]*.c
#gcc -O0 -g -c FOO_main.c
gcc -O0 FOO_main.o FOO_0*.o -o FOO_static64_$1_$2_$3
./cleaner.sh
#rm FOO_0*
#rm FOO.h FOO_main*
