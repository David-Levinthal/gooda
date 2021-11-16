#! /bin/bash
ulimit -s 65536
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator generator.c
./generator $1 $2 $3
cp driver.c FOO_driver.c
make -f Makefile -j 40
