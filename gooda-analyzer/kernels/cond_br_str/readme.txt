The conditional branch test in this directory needs some explanation:
The code was based on the walker test in this suite.
invocation:
 ./triad -h
triad driver needs at least 3 arguments, cpu_init, cpu_run, cache_level, [call count multiplier  def = 1], [offset a, offset_b, offset_c  defaults = 0] 
 argc = 2
 triad need at least 3 arguments setting values for -i, -r and -l
 -i indicates what core to use when initializing buffers
 -r indicates what core to use when running the triad
 -l value 0-3 determining the buffer size and thereby setting the cache locality of the data
 [-m] value increases the number of calls to triad by a multiplier = value
 [-a] offset in bytes to use for base address of buffer a (write target)
 [-b] offset in bytes to use for base address of buffer b (read target1)
 [-c] offset in bytes to use for base address of buffer c (read target2)

The test can be used to validate the mispredicted conditional branch count, the number of speculative instructions that are cancelled 
and thereby how speculatively executed loads and stores are handled.
the core execution is rather simple. The string_array is set to string1 or string2 (constant strings) based on the returned value of drand48 being GT 0.5
thus a 50        50 split
a nested pair of loops calls a function which has 6 arguments, an int, 2 strings and 3 double arrays
             for(k=0; k<mult; k++){
                for(i=0;i<line_count;i++){
                        str_arg = string_array[i];
                        ret_val = (size_t) triad ( len,  str_arg, string1, a, &b[i*len],  &c[i*len]);
                        total+= ret_val;
                        }
                }

it executes a very slow string comparison (which has no branches as the string lengths are known to be 16)
and then an else if block calling 2 different functions
the slow string comparison gives the mispredictions time to execute a substantial incorrect path of instructions
	 if(dumb_compare(str_arg,str_arg2) == 0){
	         bytes = FOO1(a,b,c);
	 }
	 else {
	         bytes = FOO2(a,b,c);
	 }
the functions FOO* consist of 48 explicit instructions copying 48 elements of b or c into a.
The compiler generates parallel loads and stores.
again no branches are executed
The directory constains the asm listings gcc generated, so for this case the number of asm instructions executed/inner loop iteration can be counted
the functions FOO execute 49 instructions including the ret
the dumb_compare executes 65 instructions including the ret
the "triad" executes 14 instuctions and then 6 more if the branch is correctly predicted
the inner loop of cond_br_driver_str executes 13 instructions including a call and the compare and branch (which get fused on Intel processors)
thus 2 branches are executed per inner loop iteration
and a total of 147 instructions retired/iteration.
there are 24 loads for each call to FOO, 32 in the dumb_compare and 2 in the inner loop for a total of 58/iteration. 
invoking the run_events.sh will invoke perf stat for a perf build that includes libpfm4
the code is invoked as
perf stat -a -A --pfm-events cpu_clk_unhalted,instructions_retired,uops_issued:any,uops_retired:retire_slots,mem_uops_retired:all_loads,mem_load_retired:l1_hit,
mem_load_retired:l2_hit,l2_rqsts:demand_data_rd_hit,l1d:replacement,br_inst_retired:conditional,br_misp_retired:conditional,baclears:any
 --./cond_br_driver_str -i0 -r2 -l500000 -m10000 -T0 >cond_l500k_m10000_T0_03.txt 2>&1
thus an initialized on core 0 with the loops run on core2
the inner loop had 500K iterations and the outer loop has 10k   for a total of 5 billion iterations
it take a lot of iterations to get accurate answers when using random values

the output for the counters on core 2 that I saw on an SKX server with turbo, HT and HW prefetchers enabled was:
grep "CPU2 " cond_l500k_m10000_T0_03.txt	
CPU2	1,104,578,483,384	cpu_clk_unhalted		33.33%
CPU2	740,655,292,531		instructions_retired		41.66%
CPU2	840,122,763,717		uops_issued:any			41.67%
CPU2	750,930,249,866		uops_retired:retire_slots	41.67%
CPU2	320,198,599,505		mem_uops_retired:all_loads	41.67%
CPU2	228,912,291,232		mem_load_retired:l1_hit		41.67%
CPU2	3,283,788,181		mem_load_retired:l2_hit		33.33%
CPU2	3,066,548,300		l2_rqsts:demand_data_rd_hit	33.33%
CPU2	35,150,043,369		l1d:replacement			33.33%
CPU2	10,102,812,093		br_inst_retired:conditional	33.33%
CPU2	2,497,601,620		br_misp_retired:conditional	33.33%
CPU2	12,481,388		baclears:any			33.33%

thus 5 billion iterations times 2 branches/iteration is 10 billion
50% misprediction of one of the branches yields 2.5 billion
147 instructions/iteration yields 735 billion instructions retired (did I miss one?)
there are almost 90 billion more uops issued than retired. These are the speculative instructions issued by the frontend on branch mispredictions
thus on average about 36 uops/mispredicted branch. These would almost entirely be the loads and stores in the FOO* functions.
58 loads/iteration would make us expect 290 billion loads retired which would suggest I missed 6. 
if the prefetchers are disabled the data will be pulled by the loads and the sources used for the speculative loads can be seen.
This will also show which events count speculative loads

