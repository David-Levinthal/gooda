#! /bin/bash
gcc -o generator generator.c
./generator $1 $2 $3
icc -O0 -g -fasm-blocks -c FOO_[0-9]*.c
icc -O0 -g -c FOO_main.c
icc -O0 -g FOO_main.o FOO_0*.o -o FOO_static
icc -O0 -g -fPIC -fasm-blocks -c FOO_[0-9]*.c
depth=`ls FOO_000_000_[0-9]*.o | wc -l`


for (( i = depth-1 ; i >= 0; i-- )) ; do
	j=$(( i + 1 ))
	printf -v lev3 "%03d" $i
	tail="-L. -lFOO-level$j"
	[ ! -f libFOO-level$j.so ] && tail=""
	icc --shared -o libFOO-level$i.so FOO_[0-9]*_[0-9]*_$lev3.o $tail
done
icc -o FOO_dynamic -O0 -g FOO_main.o -L. -lFOO-level0
