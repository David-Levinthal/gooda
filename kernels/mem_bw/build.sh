#! /bin/bash

which gcc
gcc -O0 -g -c triad_inl.c
gcc -g -O2 driver.c triad_inl.o -o triad_inl
gcc -O3 -funsafe-math-optimizations -g -c triad.c
gcc -g -O2 driver.c triad.o -o triad
gcc -O3 -funsafe-math-optimizations -g -mcpu=native -c triad_vec.c
gcc -g -O2 driver.c triad_vec.o -o triad_vec
gcc -O3 -funsafe-math-optimizations -g -mcpu=native -c copy_vec.c
gcc -g -O2 driver.c copy_vec.o -o copy_vec_avx
gcc -O3 -funsafe-math-optimizations -g -mcpu=native -c writer.c
gcc -g -O2 driver.c writer.o -o writer_0
gcc -O3 -funsafe-math-optimizations -g -mcpu=native -c writer10.c
gcc -g -O2 driver.c writer10.o -o writer_10
gcc -O3 -funsafe-math-optimizations -g -mcpu=native -c writer10_skip.c
gcc -g -O2 driver.c writer10_skip.o -o writer_10_skip
#gcc -O3 -funsafe-math-optimizations -g -xAVX -c writer.c
#gcc -g -O2 driver.c writer.o -o writer_avx_rfo
