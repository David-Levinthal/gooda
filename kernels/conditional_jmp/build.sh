#! /bin/sh
gcc -O0 cond_jmp.c -o generator
gcc -O0 cond_jmp64.c -o generator64
gcc -O0 cond_jmp16.c -o generator16
gcc -O0 cond_jmp_nt.c -o generator_nt
gcc -O0 cond_jmp64_nt.c -o generator64_nt
gcc -O0 cond_jmp16_nt.c -o generator16_nt
for (( i = 2 ; i < 10 ; i++ )) ; do
        j=$((i*10))
	./generator64 $j
	gcc -O0 FOO_main.c -o cjmp64_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o cjmp16_$j
	rm -rf FOO*
	./generator $j
	gcc -O0 FOO_main.c -o cjmp_$j
	rm -rf FOO*
done
for (( i = 1 ; i < 30 ; i++ )) ; do
        j=$((i*100))
	./generator64 $j
	gcc -O0 FOO_main.c -o cjmp64_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o cjmp16_$j
	rm -rf FOO*
	./generator $j
	gcc -O0 FOO_main.c -o cjmp_$j
	rm -rf FOO*
done
for (( i = 3 ; i < 10 ; i++ )) ; do
        j=$((i*1000))
	./generator64 $j
	gcc -O0 FOO_main.c -o cjmp64_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o cjmp16_$j
	rm -rf FOO*
	./generator $j
	gcc -O0 FOO_main.c -o cjmp_$j
	rm -rf FOO*
done
for (( i = 1 ; i < 10 ; i++ )) ; do
        j=$((i*10000))
	./generator64 $j
	gcc -O0 FOO_main.c -o cjmp64_$j
	rm -rf FOO*
	./generator16 $j
	gcc -O0 FOO_main.c -o cjmp16_$j
	rm -rf FOO*
	./generator $j
	gcc -O0 FOO_main.c -o cjmp_$j
	rm -rf FOO*
done


for (( i = 2 ; i < 10 ; i++ )) ; do
        j=$((i*10))
        ./generator64_nt $j
        gcc -O0 FOO_main.c -o cjmp64_nt_$j
        rm -rf FOO*
        ./generator16_nt $j
        gcc -O0 FOO_main.c -o cjmp16_nt_$j
        rm -rf FOO*
        ./generator_nt $j
        gcc -O0 FOO_main.c -o cjmp_nt_$j
        rm -rf FOO*
done
for (( i = 1 ; i < 30 ; i++ )) ; do
        j=$((i*100))
        ./generator64_nt $j
        gcc -O0 FOO_main.c -o cjmp64_nt_$j
        rm -rf FOO*
        ./generator16_nt $j
        gcc -O0 FOO_main.c -o cjmp16_nt_$j
        rm -rf FOO*
        ./generator_nt $j
        gcc -O0 FOO_main.c -o cjmp_nt_$j
        rm -rf FOO*
done
for (( i = 3 ; i < 10 ; i++ )) ; do
        j=$((i*1000))
        ./generator64_nt $j
        gcc -O0 FOO_main.c -o cjmp64_nt_$j
        rm -rf FOO*
        ./generator16_nt $j
        gcc -O0 FOO_main.c -o cjmp16_nt_$j
        rm -rf FOO*
        ./generator_nt $j
        gcc -O0 FOO_main.c -o cjmp_nt_$j
        rm -rf FOO*
done
for (( i = 1 ; i < 10 ; i++ )) ; do
        j=$((i*10000))
        ./generator64_nt $j
        gcc -O0 FOO_main.c -o cjmp64_nt_$j
        rm -rf FOO*
        ./generator16_nt $j
        gcc -O0 FOO_main.c -o cjmp16_nt_$j
        rm -rf FOO*
        ./generator_nt $j
        gcc -O0 FOO_main.c -o cjmp_nt_$j
        rm -rf FOO*
done

