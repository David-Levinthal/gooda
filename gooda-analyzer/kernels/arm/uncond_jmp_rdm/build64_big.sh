#! /bin/bash
gcc -O0 uncond_rdm_jmp64.c -o generator64
for (( i = 5 ; i < 10 ; i++ )) ; do
        j=$((i*1000))
	./generator64 $j
	gcc -O0 FOO_main.c -o rjmp64_$j
	rm -rf FOO*
done
for (( i = 1 ; i < 10 ; i++ )) ; do
        j=$((i*10000))
	./generator64 $j
	gcc -O0 FOO_main.c -o rjmp64_$j
	rm -rf FOO*
done

