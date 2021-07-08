#! /bin/bash
ulimit -s 65536
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ran generator_att_ran.c
./generator_att_ran $1 $2 $3
make -f Makefile1 -j 40
#gcc -O0 -g -c FOO_[0-9]*.c
#gcc -O0 -g -c FOO_main.c
#gcc -O0 -g FOO_main.o FOO_0*.o -o FOO_static
