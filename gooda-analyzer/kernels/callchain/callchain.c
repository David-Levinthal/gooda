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

void
usage()
{
        fprintf(stderr," callchain need at least 2 arguments setting values for -i, -m\n");
        fprintf(stderr," -i indicates what core to use when running callchain\n");
        fprintf(stderr," -m value increases the number of calls from main to the chain by a multiplier = value\n");
        fprintf(stderr," -b value sets number of iterations of a time wasting loop before the next call is executed\n");
        fprintf(stderr," -a value sets number of iterations of a time wasting loop after the next call is executed, but before the return\n");
}

int init_loop=50, final_loop=50;

int
foo21(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
//	foo22(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo20(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo21(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo19(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo20(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo18(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo19(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo17(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo18(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo16(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo17(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo15(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo16(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo14(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo15(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo13(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo14(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo12(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo13(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo11(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo12(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo10(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo11(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo9(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo10(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo8(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo9(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo7(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo8(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo6(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo7(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo5(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo6(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo4(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo5(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo3(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo4(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo2(int level, int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo3(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

int
foo1(int level,int sum)
{
	int i,j, old;
	level++;
	old = sum;

	for(i=0;i<init_loop;i++)old=old*old;
	foo2(level,old);
	for(i=0;i<final_loop;i++)old=old*old;
	return old;
}

void
main(int argc, char ** argv)
{
        double *a, *b, *c, xx=0.01, bw, avg_bw, best_bw=-1.0;
        char * buf1, *buf2, *buf3;
        int i,j,k,offset_a=0,offset_b=0,offset_c=0, mult=1,iter=100000, c_val;
        int len,level, cpu, sum;
        unsigned long long start, stop, run_time, call_start, call_stop, call_run_time,total_bytes=0;
        __pid_t pid=0;
        int cpu_setsize;
        cpu_set_t mask;


//      process input arguments

        if(argc < 3 ){
                printf("callchain needs 2 arguments, cpu_init, call count outer loop multiplier  def = 1 \n");
                printf(" argc = %d\n",argc);
                usage();
                err(1, "bad arguments");
                }


        while ((c_val = getopt(argc, argv, "i:r:l:m:a:b:c")) != -1) {
                switch(c_val) {
                case 'i':
                        cpu = atoi(optarg);
                        break;
                case 'm':
                        mult = atoi(optarg);
                        break;
                case 'a':
                        init_loop = atoi(optarg);
                        break;
                case 'b':
                        final_loop = atoi(optarg);
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

	level = 0;
	sum = 1;
	call_start = _rdtsc();
	for(i=0;i<iter; i++)
		sum+= foo1(level,sum);
	call_stop = _rdtsc();
	fprintf(stderr," a printout to fool the optimizer, sum = %d\n",sum);
	call_run_time = call_stop - call_start;
	fprintf(stderr," runtime = %lld\n",call_run_time);
}
