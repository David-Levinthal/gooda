#! /bin/bash
gcc -O0 -c dumb_compare.c
gcc -O3 -c FOO1.c
gcc -O3 -c FOO2.c
gcc -O3 -c triad.c
gcc -O3 cond_br_driver_str.c triad.o dumb_compare.o FOO1.o FOO2.o -o cond_br_driver_str
