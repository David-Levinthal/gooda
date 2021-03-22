#! /bin/sh
./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L32000 -l256 -S64 -m1 >& snoop_32K_256_i0_r2_w0.txt
grep @ snoop_32K_256_i0_r2_w0.txt > snoop_32K_256_i0_r2_w0_data.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L32000 -l2560 -S64 -m1 >& snoop_32K_2560_i0_r2_w0.txt
grep @ snoop_32K_2560_i0_r2_w0.txt > snoop_32K_2560_i0_r2_w0_data.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L32000 -l16000 -S64 -m1 >& snoop_32K_16k_i0_r2_w0.txt
grep @ snoop_32K_16k_i0_r2_w0.txt > snoop_32K_16k_i0_r2_w0_data.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L1024000 -l256 -S64 -m1 >& snoop_1M_256_i0_r2_w0.txt
grep @ snoop_1M_256_i0_r2_w0.txt > snoop_1M_256_i0_r2_w0_data.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L1024000 -l2560 -S64 -m1 >& snoop_1M_2560_i0_r2_w0.txt
grep @ snoop_1M_2560_i0_r2_w0.txt > snoop_1M_2560_i0_r2_w0_data.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L1024000 -l16000 -S64 -m1 >& snoop_1M_16k_i0_r2_w0.txt
grep @ snoop_1M_16k_i0_r2_w0.txt > snoop_1M_16k_i0_r2_w0_data.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L32000 -l256 -S64 -m1 -s >& snoop_32K_256_i0_r2_w0_s.txt
grep @ snoop_32K_256_i0_r2_w0_s.txt > snoop_32K_256_i0_r2_w0_data_s.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L32000 -l2560 -S64 -m1 -s >& snoop_32K_2560_i0_r2_w0_s.txt
grep @ snoop_32K_2560_i0_r2_w0_s.txt > snoop_32K_2560_i0_r2_w0_data_s.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L32000 -l16000 -S64 -m1 -s >& snoop_32K_16k_i0_r2_w0_s.txt
grep @ snoop_32K_16k_i0_r2_w0_s.txt > snoop_32K_16k_i0_r2_w0_data_s.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L1024000 -l256 -S64 -m1 -s >& snoop_1M_256_i0_r2_w0_s.txt
grep @ snoop_1M_256_i0_r2_w0_s.txt > snoop_1M_256_i0_r2_w0_data_s.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L1024000 -l2560 -S64 -m1 -s >& snoop_1M_2560_i0_r2_w0_s.txt
grep @ snoop_1M_2560_i0_r2_w0_s.txt > snoop_1M_2560_i0_r2_w0_data_s.txt

./fourby.sh hsw_snoop.txt 2 ./snoop_test -i0 -r2 -w0 -L1024000 -l16000 -S64 -m1 -s >& snoop_1M_16k_i0_r2_w0_s.txt
grep @ snoop_1M_16k_i0_r2_w0_s.txt > snoop_1M_16k_i0_r2_w0_data_s.txt

