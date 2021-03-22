#! /bin/sh
gcc -O0 uncond_jmp.c -o generator
gcc -O0 uncond_jmp16.c -o generator16
gcc -O0 uncond_jmpq.c -o generatorq
for (( i = 2 ; i < 10 ; i++ )) ; do
        j=$((i*10))
	./generator $j
	gcc -O0 FOO_main.c -o jmp_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o jmp16_$j
	rm -rf FOO*
        ./generatorq $j
        gcc -O0 FOO_main.c -o jmpq16_$j
        rm -rf FOO*
done
for (( i = 1 ; i < 20 ; i++ )) ; do
        j=$((i*100))
	./generator $j
	gcc -O0 FOO_main.c -o jmp_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o jmp16_$j
	rm -rf FOO*
        ./generatorq $j
        gcc -O0 FOO_main.c -o jmpq16_$j
        rm -rf FOO*
done
for (( i = 3 ; i < 10 ; i++ )) ; do
        j=$((i*1000))
	./generator $j
	gcc -O0 FOO_main.c -o jmp_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o jmp16_$j
	rm -rf FOO*
        ./generatorq $j
        gcc -O0 FOO_main.c -o jmpq16_$j
        rm -rf FOO*
done

