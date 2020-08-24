#! /bin/bash
gcc -O0 uncond_rdm_jmp16.c -o generator16
for (( i = 2 ; i < 10 ; i++ )) ; do
        j=$((i*10))
	./generator16 $j
	gcc -O0 FOO_main.c -o rjmp16_$j
	rm -rf FOO*
done
for (( i = 1 ; i < 30 ; i++ )) ; do
        j=$((i*100))
	./generator16 $j
	gcc -O0 FOO_main.c -o rjmp16_$j
	rm -rf FOO*
done
for (( i = 3 ; i < 10 ; i++ )) ; do
        j=$((i*1000))
	./generator16 $j
	gcc -O0 FOO_main.c -o rjmp16_$j
	rm -rf FOO*
done
for (( i = 1 ; i < 10 ; i++ )) ; do
        j=$((i*10000))
	./generator16 $j
	gcc -O0 FOO_main.c -o rjmp16_$j
	rm -rf FOO*
done

