#! /bin/sh

for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st16.sh $j 1000000000 > wsm-br-$j-cjmp16.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp16.txt > wsm-br-$j-cjmp16_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st16.sh $j 100000000 > wsm-br-$j-cjmp16.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp16.txt > wsm-br-$j-cjmp16_CPU4.txt
done
for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st64.sh $j 1000000000 > wsm-br-$j-cjmp64.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp64.txt > wsm-br-$j-cjmp64_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st64.sh $j 100000000 > wsm-br-$j-cjmp64.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp64.txt > wsm-br-$j-cjmp64_CPU4.txt
done
for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st.sh $j 1000000000 > wsm-br-$j-cjmp.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp.txt > wsm-br-$j-cjmp_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st.sh $j 100000000 > wsm-br-$j-cjmp.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp.txt > wsm-br-$j-cjmp_CPU4.txt
done

for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st16_nt.sh $j 1000000000 > wsm-br-$j-cjmp16_nt.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp16_nt.txt > wsm-br-$j-cjmp16_nt_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st16_nt.sh $j 100000000 > wsm-br-$j-cjmp16_nt.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp16_nt.txt > wsm-br-$j-cjmp16_nt_CPU4.txt
done
for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st64_nt.sh $j 1000000000 > wsm-br-$j-cjmp64_nt.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp64_nt.txt > wsm-br-$j-cjmp64_nt_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st64_nt.sh $j 100000000 > wsm-br-$j-cjmp64_nt.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp64_nt.txt > wsm-br-$j-cjmp64_nt_CPU4.txt
done
for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st_nt.sh $j 1000000000 > wsm-br-$j-cjmp_nt.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp_nt.txt > wsm-br-$j-cjmp_nt_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st_nt.sh $j 100000000 > wsm-br-$j-cjmp_nt.txt 2>&1
	grep CPU4 wsm-br-$j-cjmp_nt.txt > wsm-br-$j-cjmp_nt_CPU4.txt
done


