#! /bin/bash
ulimit -s 65536
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ran2 generator_att_ran2.c
./generator_att_ran2 $1 $2 $3
make -f Makefile2 -j 40
#gcc -O0 -Os -c FOO_[0-9]*.c
#gcc -O0 -Os -c FOO_main.c
#gcc -O0 -Os FOO_main.o FOO_0*.o -o FOO_static2
