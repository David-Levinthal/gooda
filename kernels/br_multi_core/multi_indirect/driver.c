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
#include <sys/types.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <getopt.h>
#include <pthread.h>

typedef unsigned long long u64;
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high)     ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)

#define MAX_THREAD 128

int exchange_flag, shared, seg_size;
size_t *array, *write_pointer, *read_pointer;
uint64_t read_sum_tsc=0, write_sum_tsc=0, total_lines,lines_in_buf; 
__pid_t pid=0;
int cpu, cpu_run;

extern int loop(int iter);

static inline unsigned long long _rdtsc(void)
{
        DECLARE_ARGS(val, low, high);

        asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));

        return EAX_EDX_VAL(val, low, high);
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

void evict_line_from_cache(volatile void *ptr)
{
    asm volatile ("clflush (%0)" :: "r"(ptr));
}

void usage()
	{
	fprintf(stderr," rndm_walker requires 6 arguments and has a seventh optional argument\n");
	fprintf(stderr," -iN  -i signifies intialization N indicates the core on which the buffer should be initialized\n");
	fprintf(stderr," -rM  -r signifies read M indicates which core the reader thread should be executed on\n");
	fprintf(stderr," -LN  -L signifies loop iterations, N indicates the number of iterations run by the outer loop\n");
	fprintf(stderr," -mN  -m signifies multiplier N indicates the multiplying factor applied to the outer loop tripcount. \n");
	}

int main(int argc, char ** argv)
{
	char * buf1, *ptr;
	void * ret;
	size_t ret_val = 0;
	size_t  array_stride;
	int rc0, rc1,flush=0;
	int i,j,k, loop_count=0,stride=0, fd = -1;
	off_t offset = 0;
	int len=1024000, iter=10,mult=1,main_ret=0;
	double iterations;
	size_t start, stop, run_time, call_start, call_stop, call_run_time,total_bytes=0;
	size_t buf_size,jj,zero_loop, buf_by_num_seg,ind;
	size_t num_pages, page_size, var_size;
	int cpu_setsize;
	cpu_set_t mask;
//	size_t pattern[] = {4,1,5,2,6,3,7,0};
	int *pattern;
	int step, c;
	int* index, lc_by_num_seg,count, num_seg=32, huge=0;
	unsigned int bitmask, *intstar;
	void *arg;

	pthread_t * Thread_dat;

	page_size = 4096;
	shared = 0;

//	process input arguments

	if(argc < 4){
		fprintf(stderr,"the multi core branch test requires at least 4 arguments (only the 5th in the list below is optional), there were %d\n",argc);
		usage();
		err(1,"insufficient invocation arguments");
		}

	while ((c = getopt(argc, argv, "i:r:w:l:S:m:L:sfh")) != -1) {
		switch(c) {
		case 'i':
			cpu = atoi(optarg);
			break;
		case 'r':
			cpu_run = atoi(optarg);
			break;
		case 'm':
			mult = atoi(optarg);
			break;
		case 'L':
			loop_count = atoi(optarg);
			break;
		case 'h':
			usage();
			exit(1);
		default:
			err(1, "unknown option %c", c);
		}
	}

// pin core affinity

	if(pin_cpu(pid, cpu) == -1) {
		err(1,"failed to set affinity");
		}
	else{
		fprintf(stderr," process pinned to core %d\n",cpu);
		}


// run the walker
	printf(" invoking reader %d times which loops  %d times on buffer of %d lines with a stride of %d, for a total size of %zu\n",iter,len,loop_count,stride,buf_size);

	iter=10;
	rc0 = loop(iter);
	if(rc0)
		err(1,"failed to start initializing loop");

// pin core affinity

	if(pin_cpu(pid, cpu_run) == -1) {
		err(1,"failed to set affinity");
		}
	else{
		fprintf(stderr," process pinned to core %d\n",cpu_run);
		}

	iter = loop_count;
	start = _rdtsc();
	rc1 = loop(iter);
	stop = _rdtsc();
	if(rc1)
		err(1,"failed to start main loop");

	call_run_time = stop - start;
//  printout
	printf(" run time = %zu\n",call_run_time);
	printf(" average cycles per iteration = %f\n", (double)call_run_time/(double)iter);
	pthread_exit(NULL);
	return main_ret;
}
