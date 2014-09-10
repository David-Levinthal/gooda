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

#include "triad.h"
#include "arch.h"


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
	fprintf(stderr," triad need at least 3 arguments setting values for -i, -r and -l\n");
	fprintf(stderr," -i indicates what core to use when initializing buffers\n");
	fprintf(stderr," -r indicates what core to use when running the triad\n");
	fprintf(stderr," -l value 0-3 determining the buffer size and thereby setting the cache locality of the data\n");
	fprintf(stderr," [-m] value increases the number of calls to triad by a multiplier = value\n");
	fprintf(stderr," [-a] offset in bytes to use for base address of buffer a (write target)\n");
	fprintf(stderr," [-b] offset in bytes to use for base address of buffer b (read target1)\n");
	fprintf(stderr," [-c] offset in bytes to use for base address of buffer c (read target2)\n");
}

	
void 
main(int argc, char ** argv)
{
	double *a, *b, *c, xx=0.01, bw, avg_bw, best_bw=-1.0;
	char * buf1, *buf2, *buf3;
	int i,j,k,offset_a=0,offset_b=0,offset_c=0, mult=1,iter=100, c_val;
	int len,mem_level, level_size[4], cpu, cpu_run, bytes_per,scale;
	unsigned long long start, stop, run_time, call_start, call_stop, call_run_time,total_bytes=0;
	__pid_t pid=0;
	int cpu_setsize;
	cpu_set_t mask;


//	process input arguments

	if(argc < 3 ){
		printf("triad driver needs at least 3 arguments, cpu_init, cpu_run, cache_level, [call count multiplier  def = 1], [offset a, offset_b, offset_c  defaults = 0] \n");
		printf(" argc = %d\n",argc);
		usage();
		err(1, "bad arguments");
		}


	len = L4;
        while ((c_val = getopt(argc, argv, "i:r:l:m:a:b:c")) != -1) {
                switch(c_val) {
                case 'i':
                        cpu = atoi(optarg);
                        break;
                case 'r':
                        cpu_run = atoi(optarg);
                        break;
                case 'l':
                        mem_level = atoi(optarg);
                        break;
                case 'm':
                        mult = atoi(optarg);
                        break;
                case 'a':
                        offset_a = atoi(optarg);
                        break;
                case 'b':
                        offset_b = atoi(optarg);
                        break;
                case 'c':
                        offset_c = atoi(optarg);
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
	level_size[0]=L1;
	level_size[1]=L2;
	level_size[2]=L3;
	level_size[3]=L4;
	fprintf(stderr, "len = %d, mem_level = %d, iter = %d, mult = %d\n",len, mem_level, iter,mult);
	len = level_size[mem_level]/32;
	fprintf(stderr, "len = %d, mem_level = %d, iter = %d, mult = %d\n",len, mem_level, iter,mult);
	scale = level_size[3]/(32*len);
	fprintf(stderr, "len = %d, mem_level = %d, iter = %d, mult = %d, scale = %d\n",len, mem_level, iter,mult,scale);
	iter =iter*scale*mult;
	
	fprintf(stderr, "len = %d, mem_level = %d, iter = %d, mult = %d\n",len, mem_level, iter,mult);

// malloc and initialize buffers
	buf1 = malloc(sizeof(double)*len + 4096 + 1024);
	fprintf(stderr," buf1 = %p\n",buf1);
	buf1 = buf1 + (0x1000 - (unsigned int)buf1 & 0xFFF) + offset_a;
	fprintf(stderr," buf1 = %p\n",buf1);
	a = (double *) buf1;
	buf2 = malloc(sizeof(double)*len + 4096 + 1024);
	fprintf(stderr," buf2 = %p\n",buf2);
	buf2 = buf2 + (0x1000 - (unsigned int)buf2 & 0xFFF) + offset_b;
	fprintf(stderr," buf2 = %p\n",buf2);
	b = (double *) buf2;
	buf3 = malloc(sizeof(double)*len + 4096 + 1024);
	fprintf(stderr," buf3 = %p\n",buf3);
	buf3 = buf3 + (0x1000 - (unsigned int)buf3 & 0xFFF) + offset_c;
	fprintf(stderr," buf3 = %p\n",buf3);
	c = (double *) buf3;

	for(i=0;i<len;i++){
		a[i] = 0.;
		b[i] = 10.;
		c[i] = 10.;
		}

// pin core affinity for triad run
        if(pin_cpu(pid, cpu_run) == -1) {
                err(1,"failed to set affinity");
                }
        else{
                fprintf(stderr," process pinned to core %d for triad run\n",cpu_run);
                }

// run the triad
	printf(" calling triad %d times with len = %d\n",iter,len);
	call_start = _rdtsc();
	for(i=0;i<iter;i++){
		start = _rdtsc();
		bytes_per = triad(len,xx,a,b,c);
		stop = _rdtsc();
		run_time = stop - start;
		xx+=0.01;
		total_bytes +=len*bytes_per;
		bw=(double)(len*bytes_per)/(double)run_time;
		if(bw > best_bw) best_bw = bw;
		}
	call_stop = _rdtsc();
	call_run_time = call_stop - call_start;
	avg_bw=(double)(total_bytes)/(double)call_run_time;
//  printout
	printf(" transfering %lld bytes from memory level %d took %lld cycles/call and a total of %lld\n",total_bytes,mem_level,run_time,call_run_time);
	printf(" average bytes/cycle = %f\n", avg_bw);
	printf(" best bytes/cycle = %f\n",best_bw);
}
