#! /bin/sh


for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st16.sh $j 1000000000 > wsm-br-$j-rjmp16.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp16.txt > wsm-br-$j-rjmp16_CPU4.txt
done

for (( i = 1 ; i < 30 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st.sh $j 100000000 > wsm-br-$j-rjmp16.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp16.txt > wsm-br-$j-rjmp16_CPU4.txt
done

for (( i = 3 ; i < 10 ; i++ )) ; do
	j=$((i*1000))
	../../run_br.sh ./run_st.sh $j 10000000 > wsm-br-$j-rjmp16.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp16.txt > wsm-br-$j-rjmp16_CPU4.txt
done

for (( i = 1 ; i < 10 ; i++ )) ; do
	j=$((i*10000))
	../../run_br.sh ./run_st.sh $j 1000000 > wsm-br-$j-rjmp16.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp16.txt > wsm-br-$j-rjmp16_CPU4.txt
done

for (( i = 2 ; i < 10 ; i++ )) ; do
	j=$((i*10))
	../../run_br.sh ./run_st64.sh $j 1000000000 > wsm-br-$j-rjmp64.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp64.txt > wsm-br-$j-rjmp64_CPU4.txt
done

for (( i = 1 ; i < 20 ; i++ )) ; do
	j=$((i*100))
	../../run_br.sh ./run_st64.sh $j 100000000 > wsm-br-$j-rjmp64.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp64.txt > wsm-br-$j-rjmp64_CPU4.txt
done

for (( i = 3 ; i < 10 ; i++ )) ; do
	j=$((i*1000))
	../../run_br.sh ./run_st64.sh $j 10000000 > wsm-br-$j-rjmp64.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp64.txt > wsm-br-$j-rjmp64_CPU4.txt
done

for (( i = 1 ; i < 10 ; i++ )) ; do
	j=$((i*10000))
	../../run_br.sh ./run_st.sh $j 1000000 > wsm-br-$j-rjmp64.txt 2>&1
	grep CPU4 wsm-br-$j-rjmp64.txt > wsm-br-$j-rjmp64_CPU4.txt
done


