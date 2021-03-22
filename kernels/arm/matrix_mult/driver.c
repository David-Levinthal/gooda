/*
Copyright 2012 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#include <sys/mman.h>
#include <asm/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <getopt.h>

#include "matrix_mult.h"


typedef unsigned long long u64;
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define isb()           asm volatile("isb" : : : "memory")

static inline u64 _rdtsc(void)
{
        u64 cval;

        isb();
        asm volatile("mrs %0, cntvct_el0" : "=r" (cval));

        return cval;
}

static inline u64 _loc_freq(void)
{
        u64 cval;

        isb();
        asm volatile("mrs %0, cntfrq_el0" : "=r" (cval));

        return cval;
}

#define MAX_CPUS        2048
#define NR_CPU_BITS     (MAX_CPUS>>3)
static int
pin_cpu(pid_t pid, unsigned int cpu)
{
       unsigned long long my_mask[NR_CPU_BITS];

       memset(my_mask, 0, sizeof(my_mask));

       if (cpu >= MAX_CPUS)
               errx(1, "this program supports only up to %d CPUs", MAX_CPUS);

       my_mask[cpu>>6] = 1ULL << (cpu&63);

       return syscall(__NR_sched_setaffinity, pid, sizeof(my_mask), &my_mask);
}

void
usage()
{
	fprintf(stderr," matrix_mult needs 3 argument setting init core (i), run core (-rY) and size of square matrix (-l)\n");
	fprintf(stderr," -i indicates what core to use when initializing buffers\n");
	fprintf(stderr," -r indicates what core to use when running the triad\n");
	fprintf(stderr," -l value sets dimension of square matrix, buffer size = l*l*8 \n");
	fprintf(stderr," [-m] value increases the number of calls to triad by a multiplier = value\n");
}

	
void 
main(int argc, char ** argv)
{
	double *a, *b, *c, xx=0.01, bw, avg_bw, best_bw=-1.0;
	char * buf1, *buf2, *buf3;
	int i,j,k,offset_a=0,offset_b=0,offset_c=0, mult=1,iter=1, c_val;
	int len,mem_level, level_size[4], cpu, cpu_run,scale;
	__pid_t pid=0;
	int cpu_setsize;
	cpu_set_t mask;
        size_t start, stop, run_time, call_start, call_stop, call_run_time,total_flops=0, flops_per;
        size_t sum_run_time=0, run_time_k=0, init_start, init_end, init_run;
        struct timespec start_t, stop_t;
        size_t total=0;
        struct timeval start_time, stop_time;
        int ret_int, fd = -1;
        size_t gotten_time, freq_val;
        off_t offset = 0;
	size_t buf_size;

//	process input arguments

	if(argc < 3 ){
		printf("triad driver needs at least 3 arguments, cpu_init, cpu_run, cache_level, [call count multiplier  def = 1], [offset a, offset_b, offset_c  defaults = 0] \n");
		printf(" argc = %d\n",argc);
		usage();
		err(1, "bad arguments");
		}


	len = 2048;
        while ((c_val = getopt(argc, argv, "i:r:l:m:a:b:c")) != -1) {
                switch(c_val) {
                case 'i':
                        cpu = atoi(optarg);
                        break;
                case 'r':
                        cpu_run = atoi(optarg);
                        break;
/*                case 'l':
                        len = atoi(optarg);
                        break;
*/
		case 'm':
                        mult = atoi(optarg);
                        break;
                default:
                        err(1, "unknown option %c", c_val);
                }
        }
        iter = iter*mult;



// pin core affinity for initialization
        if(pin_cpu(pid, cpu) == -1) {
                err(1,"failed to set affinity");
                }
        else{
                fprintf(stderr," process pinned to core %d for initialization\n",cpu);
                }


// set buffer sizes and loop tripcounts based on memory level
	buf_size = (len+8)*(len+8)*8;
	fprintf(stderr, "len = %d, buf_szie = %zd, iter = %d, mult = %d\n",len, buf_size, iter,mult);

// malloc and initialize buffers
	buf1 = (char*) mmap(NULL,buf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	buf2 = (char*) mmap(NULL,buf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	buf3 = (char*) mmap(NULL,buf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	a = (double *) buf1;
	b = (double *) buf2;
	c = (double *) buf3;

	for(i=0;i<len;i++){
		for(k=0; k<len;k++){
			a[i*len+k] = 0.;
			b[i*len+k] = 1.1;
			c[i*len+k] = 1.1;
		}
	}

// pin core affinity for triad run
        if(pin_cpu(pid, cpu_run) == -1) {
                err(1,"failed to set affinity");
                }
        else{
                fprintf(stderr," process pinned to core %d for triad run\n",cpu_run);
                }

// run the triad
	printf(" calling matrix_mult %d times with dimension len = %d\n",iter,len);
        ret_int = gettimeofday(&start_time, NULL);
	call_start = _rdtsc();
	for(i=0;i<iter;i++){
		start = _rdtsc();
		flops_per = matrix_mult(len,a,b,c);
		stop = _rdtsc();
		run_time = stop - start;
		xx+=0.01;
		total_flops +=flops_per;
		bw=(double)(flops_per)/(double)run_time;
		if(bw > best_bw) best_bw = bw;
		}
	call_stop = _rdtsc();
        freq_val = _loc_freq();
        ret_int = gettimeofday(&stop_time, NULL);
        gotten_time = (size_t) (stop_time.tv_sec - start_time.tv_sec)*1000000;
        gotten_time += (size_t)(stop_time.tv_usec - start_time.tv_usec);

	call_run_time = call_stop - call_start;
	avg_bw=(double)(total_flops)/(double)call_run_time;
//  printout
	printf(" total %zd flops %d took %zd cycles/call and a total of %zd\n",total_flops,mem_level,run_time,call_run_time);
        printf(" run time = %zd\n",call_run_time);
        printf(" gettimeofday_run time = %zd\n",gotten_time);
        printf("frequency = %zd\n",freq_val);

	printf(" average flops/cycle = %f\n", avg_bw);
	printf(" best flops/cycle = %f\n",best_bw);
}
