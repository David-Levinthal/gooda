#! /bin/sh
ulimit -n 32768
lines=0
        perf stat -A -C2 --pfm-events MEM_LOAD_L3_MISS_RETIRED:LOCAL_DRAM:u,MEM_LOAD_RETIRED:L1_HIT:u,MEM_LOAD_RETIRED:L2_HIT:u,MEM_LOAD_RETIRED:L3_HIT:u -- ./latency_noarch/walker_nop -i0 -r2 -l256 -s0 -S1
for i in {0..100}; do
        let lines=lines+512
        perf stat -A -C2 --pfm-events MEM_LOAD_L3_MISS_RETIRED:LOCAL_DRAM:u,MEM_LOAD_RETIRED:L1_HIT:u,MEM_LOAD_RETIRED:L2_HIT:u,MEM_LOAD_RETIRED:L3_HIT:u -- ./latency_noarch/walker_nop -i0 -r2 -l$lines -s0 -S1
done
lines=0
for i in {0..100}; do
        let lines=lines+65536
        perf stat -A -C2 --pfm-events MEM_LOAD_L3_MISS_RETIRED:LOCAL_DRAM:u,MEM_LOAD_RETIRED:L1_HIT:u,MEM_LOAD_RETIRED:L2_HIT:u,MEM_LOAD_RETIRED:L3_HIT:u -- ./latency_noarch/walker_nop -i0 -r2 -l$lines -s0 -S1
done
lines=0
for i in {0..20}; do
        let lines=lines+8388608
        perf stat -A -C2 --pfm-events MEM_LOAD_L3_MISS_RETIRED:LOCAL_DRAM:u,MEM_LOAD_RETIRED:L1_HIT:u,MEM_LOAD_RETIRED:L2_HIT:u,MEM_LOAD_RETIRED:L3_HIT:u -- ./latency_noarch/walker_nop -i0 -r2 -l$lines -s0 -S1
done

