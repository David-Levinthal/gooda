/*
Copyright 2014 Google Inc. All Rights Reserved.

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

#define _GNU_SOURCE
#include <sched.h>
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
#include <getopt.h>
#include <pthread.h>

typedef unsigned long long u64;
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high)     ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)
#define MAX_THREAD 128

int exchange_flag, shared, seg_size;
uint64_t read_sum_tsc=0, iterations;
int cpu, cpu_read, cpu_write;
unsigned long long initial_value[6];
int start_loop = 0;

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

struct thread_data {
	pthread_t	thread_id;
	int		thread_num;
	uint64_t*	arg_array;
	};

void * driver0(void * arg)
{
	int i,j,k, iter_count =0, num_args=7, my_cpu, ret_val;
	uint64_t line_count=0, init_tsc, end_tsc, arg_addr;
        uint64_t msk1,msk2,msk3,msk4,msk5,msk6, limit;
	unsigned int outval;
	pthread_t tid=0;
       unsigned long long my_mask[NR_CPU_BITS];

	struct thread_data *tdata;
	cpu_set_t cpuset;

	fprintf(stderr,"entering thread\n");
	limit = iterations;
	tdata = (struct thread_data *) arg; 

//	fprintf(stderr," & arg = %p\n",arg);
//	fprintf(stderr," thread_id = %lx, thread_num = %d, addr of thread_id = %p\n",tdata->thread_id, tdata->thread_num,&tdata->thread_id);
//	arg_addr = &tdata->arg_array[0];
//	fprintf(stderr," arg addr = %lx\n",arg_addr);

//        fprintf(stderr,"from thread %d arguments ",my_cpu);
//        for(j=0; j < num_args; j++)
//                fprintf(stderr,"  %lx,  ",tdata->arg_array[j]);
//        fprintf(stderr,"\n");
	msk1 = tdata->arg_array[1];
	msk2 = tdata->arg_array[2];
	msk3 = tdata->arg_array[3];
	msk4 = tdata->arg_array[4];
	msk5 = tdata->arg_array[5];
	msk6 = tdata->arg_array[6];

// pin core affinity
	my_cpu = tdata->arg_array[0];
	tid = tdata->thread_id;

	CPU_ZERO(&cpuset);
	CPU_SET(my_cpu, &cpuset);
	fprintf(stderr," thread_id = %lx, my_cpu = %d\n",tid,my_cpu);
	ret_val = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	fprintf(stderr," thread_id = %lx, my_cpu = %d, sched ret_val = %d\n",tid,my_cpu,ret_val);
/*
	if(pin_cpu(pid, my_cpu) == -1) {
		err(1,"cannot set cpu read affinity for core", my_cpu);
		}
	else{
		printf(" read thread pinned to core %d to run\n",my_cpu);
		}
*/
	fprintf(stderr,"iterations = %ld\n",iterations);

	read_sum_tsc = 0;
	while (start_loop == 0){ }
	fprintf(stderr," thread %d starts loop, tripcount = %ld\n",my_cpu,limit);
/*
	while(iter_count < iterations)
		{
		iter_count++;
		}
*/
	asm( 
	"movq %2, %%r9  \n\t"
	"movq %2, %%r10  \n\t"
	"movq %2, %%r11  \n\t"
	"movq %2, %%r12  \n\t"
	"movq %2, %%r13  \n\t"
	"movq %2, %%r14  \n\t"
	"movq %1, %%r15  \n\t"
	"xorq %%rdx, %%rdx \n\t"
	"LOOP:       \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"xorq %%r9, %%r10   \n\t"
	"xorq %%r11, %%r12  \n\t"
	"xorq %%r13, %%r14  \n\t"
	"inc %%rdx  \n\t"
	"cmp %%r15, %%rdx   \n\t"
	"jl LOOP  \n\t"
	"movl %%edx, %0  \n\t"
	: "=r"(outval)
	: "r"(limit), "r"(msk1)
	: "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "%rdx"
	);
	pthread_exit(NULL);
}


void usage()
	{
	fprintf(stderr," power_drive requires 2 arguments\n");
	fprintf(stderr," -nN  -n sets the number of physical cores/socket\n");
	fprintf(stderr," -tN  -t sets the number of threads/core\n");
	fprintf(stderr," -sN  -s sets the number of sockets\n");
	fprintf(stderr," -mN  -m sets the multiplier of the default number of iterations\n");
	fprintf(stderr," -h  generates this output\n");
	}

int main(int argc, char ** argv)
{
	char * buf1;
	void * ret;
	size_t ret_val = 0;
	uint64_t iter = 5000000000;   //5 billion
	int rc0, rc1,num_args = 7;
	int i,j,k, line_count=0,stride=0, fd = -1;
	int num_phys_cores_per_socket=0, num_thread_per_core=0, num_sockets=0;
	int num_threads;
	off_t offset = 0;
	int len=10240000,mult=1,main_ret=0;
	size_t start, stop, run_time, call_start, call_stop, call_run_time,total_bytes=0;
	size_t buf_size,jj,zero_loop, buf_by_num_seg,ind;
	size_t num_pages, page_size, var_size;
	int cpu_setsize;
	cpu_set_t mask;
	int *pattern;
	int step, c;
	int* index, lc_by_num_seg,count, num_seg=32, huge=0;
	unsigned int bitmask, *intstar;
	void *arg;
	uint64_t ** arg_array;

	struct thread_data * Thread_dat;
	pthread_attr_t attr;

	page_size = 4096;
	shared = 0;

//	process input arguments

	if(argc < 2){
		fprintf(stderr,"the power test requires at least 2 arguments, there were %d\n",argc);
		usage();
		err(1,"insufficient invocation arguments");
		}

	while ((c = getopt(argc, argv, "n:t:s:m:h")) != -1) {
		switch(c) {
		case 'n':
			num_phys_cores_per_socket = atoi(optarg);
			break;
		case 't':
			num_thread_per_core = atoi(optarg);
			break;
		case 's':
			num_sockets = atoi(optarg);
			break;
		case 'm':
			mult = atoi(optarg);
			break;
		case 'h':
			usage();
			exit(1);
		default:
			err(1, "unknown option %c", c);
		}
	}
	iterations = iter*mult;

	var_size = sizeof(size_t);
	num_threads = num_phys_cores_per_socket*num_thread_per_core*num_sockets;
	fprintf(stderr," num_threads = %d\n",num_threads);

//	Initialize thread creation strucs etc

	c = pthread_attr_init(&attr);
	if (c != 0) err(1," pthread_attr_init error");

 

	Thread_dat =  calloc(num_threads, sizeof(struct thread_data));
	if(Thread_dat == 0)err(1,"malloc of Thread_dat failed");
        arg_array = (uint64_t **) malloc(num_threads*sizeof(uint64_t *));
	if(arg_array == 0)err(1,"malloc of arg_array failed");
	for(i=0; i< num_threads; i++)
		{
		arg_array[i] = (uint64_t *) malloc(num_args*sizeof(uint64_t));
		if(arg_array[i] == 0)err(1,"failed to malloc arg array for thread %d",i);
		arg_array[i][0] = (uint64_t) i;
		for(j=1; j < num_args; j++)arg_array[i][j] = 0xf0f0f0f0f0f0f0f0UL;
		fprintf(stderr,"thread %d arguments ",i);
		for(j=0; j < num_args; j++)
			fprintf(stderr,"  %lx,  ",arg_array[i][j]);
		fprintf(stderr,"\n");
		Thread_dat[i].thread_num = i+1;
		Thread_dat[i].arg_array = &arg_array[i][0];
		}

//	exit(1);
// pin core affinity
/*
	if(pin_cpu(pid, cpu) == -1) {
		err(1,"failed to set affinity");
		}
	else{
		fprintf(stderr," process pinned to core %d\n",cpu);
		}

*/
// run the threads

	exchange_flag = 0;
	for(i=0; i<num_threads; i++)
		{
		arg =(void*)&arg_array[i];
		fprintf(stderr," about to call pthread_create for thread = %lx, loop index = %d, thread_dat.arg_array[0] = %lx, addr of Thread_dat[i] = %p\n",arg_array[i][0],i, Thread_dat[i].arg_array[0], &Thread_dat[i]);
		rc0 = pthread_create(&Thread_dat[i].thread_id, &attr, driver0, &Thread_dat[i]);
		if(rc0)
			err(1,"failed to start thread for driver0 for thread %d", i);
		}
	start_loop = 1;
//  printout
	printf(" average cycles per call = %f\n", (double)call_run_time);
	pthread_exit(NULL);
	return main_ret;
}
