#! /bin/sh

which icc
icc -O0 -g -c triad_inl.c
icc -g -O2 driver.c triad_inl.o -o triad_inl
icc -O3 -fno-alias -g -c triad.c
icc -g -O2 driver.c triad.o -o triad
icc -O3 -fno-alias -g -xSSE4.2 -c triad_vec.c
icc -g -O2 driver.c triad_vec.o -o triad_vec
icc -O3 -fno-alias -g -xAVX -c triad_vec.c
icc -g -O2 driver.c triad_vec.o -o triad_vec_avx
icc -O3 -fno-alias -g -xSSE4.2 -c triad.c
icc -g -O2 driver.c triad.o -o triad_sse_rfo
icc -O3 -fno-alias -g -xAVX -c triad.c
icc -g -O2 driver.c triad.o -o triad_avx_rfo
