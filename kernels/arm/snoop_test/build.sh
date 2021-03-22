#! /bin/sh
gcc -c -O0 kernels.c
gcc -O0 -static snoop_test.c kernels.o -lpthread -o snoop_test
