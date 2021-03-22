#! /bin/bash
set -x
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
gcc -o generator_att_ind generator_att_ind.c
./generator_att_ind $1 $2 $3
gcc -O0 -g -c FOO_[0-9]*.c
gcc -O0 -g -c FOO_main.c
gcc -O0 -g FOO_main.o FOO_0*.o -o FOO_static
gcc -O0 -g -fPIC -c FOO_[0-9]*.c
depth=`ls FOO_000_000_[0-9]*.o | wc -l`
tail2=" "

for (( i = depth-1 ; i >= 0; i-- )) ; do
	j=$(( i + 1 ))
	printf -v lev3 "%03d" $i
	tail="-L. -lFOO-level$j"
	tmp=$tail2"-lFOO-level$i "
	tail2=$tmp
	[ ! -f libFOO-level$j.so ] && tail=""
	gcc --shared -o libFOO-level$i.so FOO_[0-9]*_[0-9]*_$lev3.o -Wl,--copy-dt-needed-entries $tail
done
gcc -o FOO_dynamic -O0 -g FOO_main.o -Wl,--copy-dt-needed-entries -L. $tail2 
