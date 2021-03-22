#! /bin/sh
gcc -O0 linear_gen_full.c -o generator_full
for (( i = 1 ; i <31 ; i++ )) ; do
        j=$((i*100))
        ./generator_full $j 1
        gcc -O0 FOO_main.c -o linear_full_$j
        rm -rf FOO*
done
./generator_full 4000 1
gcc -O0 FOO_main.c -o linear_full_4000
rm FOO*
./generator_full 40000 1
gcc -O0 FOO_main.c -o linear_full_40000
rm FOO*
./generator_full 800000 1
gcc -O0 FOO_main.c -o linear_full_800000
rm FOO*
./generator_full 1600000 1
gcc -O0 FOO_main.c -o linear_full_1600000
rm FOO*
