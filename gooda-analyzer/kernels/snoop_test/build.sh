#! /bin/sh
gcc -c -O0 kernel2.c
gcc -O0 -static snoop_test.c kernel2.o -lpthread -o snoop_test
