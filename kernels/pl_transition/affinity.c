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
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <getopt.h>



typedef unsigned long long u64;
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high)     ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)
static inline unsigned long long 
_rdtsc(void)
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

void
usage()
{
	fprintf(stderr," malloc_test need at least 3 arguments setting values for -r, -m and -l\n");
	fprintf(stderr," -r indicates what core to use when running the triad\n");
	fprintf(stderr," [-m] value increases the number of calls to triad by a multiplier = value\n");
}

	
void 
main(int argc, char ** argv)
{
	double *a, *b, *c, xx=0.01, bw, avg_bw, best_bw=-1.0;
	char * buf1, *buf2, *buf3;
	int i,j,k,offset_a=0,offset_b=0,offset_c=0, mult=1,iter=1000, c_val;
	int len,num_pages, num_lines, cpu_run,scale;
	u64 start, stop, run_time, call_start, call_stop, call_run_time,total_bytes=0;
	__pid_t pid=0;
	int cpu_setsize;
	cpu_set_t mask;
	int *buff;
	size_t buf_size;
	off_t offset = 0;
	int fd = -1;

//	process input arguments

	if(argc < 3 ){
		printf("affinity needs 2 arguments, cpu_run, call count multiplier  def = 1\n");
		printf(" argc = %d\n",argc);
		usage();
		err(1, "bad arguments");
		}


        while ((c_val = getopt(argc, argv, "i:r:l:m:a:b:c")) != -1) {
                switch(c_val) {
                case 'r':
                        cpu_run = atoi(optarg);
                        break;
                case 'm':
                        mult = atoi(optarg);
                        break;
                default:
                        err(1, "unknown option %c", c_val);
                }
        }



// pin core affinity for initialization
        if(pin_cpu(pid, cpu_run) == -1) {
                err(1,"failed to set affinity");
                }
        else{
                fprintf(stderr," process pinned to core %d for triad run\n",cpu_run);
                }


// set buffer sizes and loop tripcount
	buf_size = (u64)4096*(u64)num_pages;
	num_lines=64*num_pages;
        iter = iter*mult;

// malloc and initialize buffers

	printf(" starting malloc loop of %d iterations with buf_size = %ld, num_lines = %d\n",iter,buf_size, num_lines);
	call_start = _rdtsc();
	for(i=0;i<iter;i++){
		start = _rdtsc();
	        if(pin_cpu(pid, cpu_run) == -1) {
        	        err(1,"failed to set affinity");
                	}
	        else{
        	        fprintf(stderr," process pinned to core %d for triad run\n",cpu_run);
                	}
		stop = _rdtsc();
		run_time = stop - start;
		}
	call_stop = _rdtsc();
	call_run_time = call_stop - call_start;
//  printout
	printf(" allocating %lld bytes and initializing and freeing took %lld cycles\n",(u64)len*(u64)iter,run_time);
}
