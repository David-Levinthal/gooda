#! /bin/sh
gcc -O0 linear_gen_full2.c -o generator_full2
for (( i = 1 ; i <31 ; i++ )) ; do
        j=$((i*100))
        ./generator_full2 $j 1
        gcc -O0 FOO_main.c -o linear_full2_$j
        rm -rf FOO*
done
./generator_full2 4000 1
gcc -O0 FOO_main.c -o linear_full2_4000
rm FOO*
./generator_full2 40000 1
gcc -O0 FOO_main.c -o linear_full2_40000
rm FOO*
./generator_full2 800000 1
gcc -O0 FOO_main.c -o linear_full2_800000
rm FOO*
./generator_full2 1600000 1
gcc -O0 FOO_main.c -o linear_full2_1600000
rm FOO*
