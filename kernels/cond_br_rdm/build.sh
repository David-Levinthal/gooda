#! /bin/bash
gcc -c -O3 br_kernel.c
gcc -O3 cond_br_rdm.c br_kernel.o -o br_kernelO3 -lm 
gcc -c -O0 br_kernel.c
gcc -O3 cond_br_rdm.c br_kernel.o -o br_kernelO0 -lm 
