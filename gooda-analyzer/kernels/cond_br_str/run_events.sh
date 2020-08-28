#! /bin/bash
perf stat -a -A --pfm-events cpu_clk_unhalted,instructions_retired,uops_issued:any,uops_retired:retire_slots,mem_uops_retired:all_loads,mem_load_retired:l1_hit,mem_load_retired:l2_hit,l2_rqsts:demand_data_rd_hit,l1d:replacement,br_inst_retired:conditional,br_misp_retired:conditional,baclears:any -- ./cond_br_driver_str  -i0 -r2 -l500000 -m10000 -T0 >cond_l500k_m10000_T0_03.txt 2>&1
