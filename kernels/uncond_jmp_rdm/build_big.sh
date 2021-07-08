#! /bin/bash
gcc -O0 uncond_rdm_jmp64.c -o generator64
gcc -O0 uncond_rdm_jmp16.c -o generator16
for (( i = 18 ; i < 20 ; i++ )) ; do
        j=$((i*10000))
	./generator64 $j
	gcc -O0 FOO_main.c -o rjmp64_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o rjmp16_$j
	rm -rf FOO*
done

