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

#include "triad.h"

typedef unsigned long long u64;
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high)     ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)
#define PRIME_VAL 500009
#define ARRAY_LEN 100
#define MAX_DEPTH 90000

double drand48(void);
struct timespec current_kernel_time(void);

#define MAX_CPUS        2048
#define NR_CPU_BITS     (MAX_CPUS>>3)

static inline unsigned long long _rdtsc(void)
{
        DECLARE_ARGS(val, low, high);

        asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));

        return EAX_EDX_VAL(val, low, high);
}
	
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

void rndm_list(int* list, int n)
{
	int* l1,*l2;
	int i,j,k, count, count2;
	double val, tmp;

	val = 0.;
	l1 = (int*)malloc(n*sizeof(int));
	l2 = (int*)malloc(n*sizeof(int));
	for(i=0;i<100;i++)val+=drand48();
	for(i=0;i<n-1;i++)
		{
		l1[i] = i+1;
		l2[i] = -1;
		}
	count = n-1;
	for(i=0; i<n-1; i++)
		{
		val = drand48()*(double)count;
		j = (int) val;
//	this should never happen...but...
		while(j == count+1)
			{
			val = drand48()*(double)count;
			j = val;
			}
		list[i] = l1[j];
		l1[j] = -1;
		count2 = 0;
		for(k=0; k< count; k++)
			{
			if(l1[k] != -1)
				{
				l2[count2] = l1[k];
				count2++;
				}
			}
		count--;
		for(k=0;k<count;k++)
			{
			l1[k] = l2[k];
			}
		}
	list[n-1] = 0;
//	for(k=0;k<n;k++)printf(" k = %d, list[k] = %d",k,list[k]);
//	printf("\n");
}


void usage()
	{
	fprintf(stderr," rndm_walker requires 6 arguments and has a seventh optional argument\n");
	fprintf(stderr," -iN  -i signifies intialization N indicates the core on which the buffer should be initialized\n");
	fprintf(stderr," -rM  -r signifies run M indicates which core the pointer walker should be executed on\n");
	fprintf(stderr," -lN  -l signifies inner loop depth N indicates the number of calls to the top of the conditional branch test kernel,\n");
        fprintf(stderr," this size controls the buffer size and thus in which level of cache/memory the buffers reside. \n");
	fprintf(stderr," The HW prefetchers will pull the buffers to L1d unless they are disabled. When disabled one can test speculative load counts by level\n");
	fprintf(stderr," [-m] value increases the number of calls to triad by a multiplier = value\n");
	fprintf(stderr," [-T] allowed values 0,1,2, default is 0 meaning random test arg, 1 means test_arg = 1 2 means test arg = 2 \n");
	}

int main(int argc, char ** argv)
{
	char *buf1, *buf2, *buf3;
        char triad_name[100], triad_base[]="triad_";
	char * string_array[PRIME_VAL], *str_arg;
	int triad_base_len = 6, C;
	void * ret;
	size_t * array, *pnt, element, ret_val = 0;
	size_t  array_stride;
	int i,j,k,cpu,cpu_run,line_count,stride, fd = -1, br_path;
	off_t offset = 0;
	int len=10240000, iter=100,mult=1,test_val=0,main_ret=0;
	double iterations, br_path_val[PRIME_VAL], *a, *b, *c, ranval;
	size_t start, stop, run_time, call_start, call_stop, call_run_time,total_bytes=0;
	size_t sum_run_time=0, run_time_k=0, init_start, init_end, init_run;
	__pid_t pid=0;
	size_t buf_size1,buf_size23,jj,zero_loop, buf_by_num_seg,ind;
	size_t num_pages, page_size, var_size, line_size;
	int cpu_setsize;
	cpu_set_t mask;

	int *pattern;
	int step, bad,counter;
	int* index, *index_test, lc_by_num_seg,count, num_seg=32, huge=0;
	unsigned int bitmask, *intstar;
	struct timespec start_t, stop_t;
	size_t total=0;
	struct timeval start_time, stop_time;
	int ret_int;
	size_t gotten_time, freq_val;

	typedef long (*triad_pt) (int len, char* str_arg, char* str_arg2, double *restrict a, double *restrict b, double *restrict c);
        triad_pt *triad_array;

	char string1[1024], string2[1024];

	strncpy(string1,"this_is_string_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa_1",1024);
	strncpy(string2,"this_is_string_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa_2",1024);
/*
	fprintf(stderr, " string1 = %s\n",string1);
	fprintf(stderr, " string2 = %s\n",string2);
*/
	fprintf(stderr," string length = %ld\n",strlen(string1));
	page_size = 4096;
	line_size = 64;
	len = ARRAY_LEN;


	init_start = _rdtsc();
//	process input arguments

	if(argc < 3){
		fprintf(stderr,"the random conditional branch code requires at least 3 arguments  there were %d\n",argc);
		usage();
		err(1,"insufficient invocation arguments");
		}

	while ((C = getopt(argc, argv, "i:r:l:m:T:C")) != -1) {
		switch(C) {
		case 'i':
			cpu = atoi(optarg);
			break;
		case 'r':
			cpu_run = atoi(optarg);
			break;
		case 'l':
			line_count = atoi(optarg);
			break;
                case 'm':
                        mult = atoi(optarg);
                        break;
                case 'T':
                        test_val = atoi(optarg);
                        break;
		default:
			err(1, "unknown option %c", C);
		}
	}
	if((test_val != 0) && (test_val != 1) && (test_val !=2)){
		fprintf(stderr, "invalid value for -T was %d must be 0 or 1 or 2\n",test_val);
		exit(1);
		}
//	fprintf(stderr,"got arguments\n");


	var_size = sizeof(size_t);
	fprintf(stderr, "size_t in %zd bytes\n",var_size);
// pin core affinity

	if(pin_cpu(pid, cpu) == -1) {
		err(1,"failed to set affinity");
		}
	else{
		fprintf(stderr," process pinned to core %d\n",cpu);
		}


	iterations = (double)iter;
//    create index array for "random" pattern
	index = (int*)malloc(line_count*sizeof(int));
	if(index == NULL)
		{
		fprintf(stderr," failed to malloc index array for line_count of %d\n",line_count);
		err(1,"failed to malloc index");
		}
//	fprintf(stderr," calling rndm_list, line_count = %d\n",line_count);
//	rndm_list(index,line_count);

//      make triad_array
//	fprintf(stderr," calling make_array, line_count = %d, MAX_DEPTH = %d\n",line_count,MAX_DEPTH);
//	triad_array = (triad_pt *) make_array(MAX_DEPTH);
//	fprintf(stderr," creating br_path_val and string_array , prime_val = %d\n",PRIME_VAL);

	for(i=0; i<line_count; i++){
		ranval = drand48();
		if(test_val == 1)string_array[i] = string1;
		if(test_val == 2)string_array[i] = string2;
		if(test_val == 0){
			string_array[i] = string1;
			if(ranval > 0.5)string_array[i] = string2;
			}
		}
/*
	for(i=0; i<10; i++)fprintf(stderr, "i= %d, string[%d] = %s\n",i,i,string_array[i]);
        counter = 0;
	for(i=0; i<line_count; i++) if(strcmp(string_array[i],string2) == 0)counter++;
	fprintf(stderr," counter = %d\n",counter);
*/
//	exit(1);
//	fprintf(stderr," creating a buffer , buffer1_size = %zd\n",buf_size1);

	buf_size1 = len*sizeof(double);
	buf1 = (char*) mmap(NULL,buf_size1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	if(buf1 == NULL)
		{
		fprintf(stderr," failed to malloc buf1 for len*sizeof(double) of %zd\n",buf_size1);
		err(1,"failed to malloc buf1");
		}
//	fprintf(stderr," creating b,c buffers , buffer23_size = %zd\n",buf_size23);
	buf_size23 = line_count*len*sizeof(double);
	buf2 = (char*) mmap(NULL,buf_size23, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	if(buf2 == NULL)
		{
		fprintf(stderr," failed to malloc buf2 for line_count*len*sizeof(double) of %zd\n",buf_size23);
		err(1,"failed to malloc buf2");
		}
	buf3 = (char*) mmap(NULL,buf_size23, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	if(buf1 == NULL)
		{
		fprintf(stderr," failed to malloc buf3 for line_count*len*sizeof(double) of %zd\n",buf_size23);
		err(1,"failed to malloc buf3");
		}

	a = (double*)buf1;
	b = (double*)buf2;
	c = (double*)buf3;

//	fprintf(stderr," initializing a buffers \n");
	for(i=0;i<len;i++)a[i]=0;
//	fprintf(stderr," initializing b,c buffers \n");
	for(i=0;i<line_count*len;i++)b[i]=1;
	for(i=0;i<line_count*len;i++)c[i]=2;

	init_end = _rdtsc();
// pin core affinity
	if(pin_cpu(pid, cpu_run) == -1) {
		err(1,"cannot set cpu run affinity");
		}
	else{
		printf(" process pinned to core %d to run\n",cpu_run);
		}

// run the conditional branch loop
	printf(" calling conditional br loop %d times which loops  %d times on set of %d functions\n",mult,iter,line_count);
	call_start = _rdtsc();
	ret_int = gettimeofday(&start_time, NULL);
	j=0;
//	for(j=0; j<100; j++){
//	     for(k=0; k<mult/100; k++){
	     for(k=0; k<mult; k++){
		for(i=0;i<line_count;i++){
//			start = _rdtsc();
//		br_path=0;
//		if(br_path_val[j] < 0.5)br_path=1;
			str_arg = string_array[i];
//	                j++;
//			if(j >= PRIME_VAL)j=0;
//			fprintf(stderr," str_arg = %s, i= %d, j = %d, index[j] = %d\n",
//			       str_arg, i,j,index[i]);	
			ret_val = (size_t) triad ( len,  str_arg, string1, a, &b[i*len],  &c[i*len]);
			total+= ret_val;
//	fprintf(stderr, " retval = %ld\n",ret_val);

//			stop = _rdtsc();
//		run_time = (stop_t.tv_sec - start_t.tv_sec)*1000000000;
//		run_time_k += stop_t.tv_nsec - start_t.tv_nsec;
//			sum_run_time += stop - start;
			}
		}
//	     fprintf(stderr," j = %d\n",j);
//	}
	printf(" done\n");
	call_stop = _rdtsc();
	ret_int = gettimeofday(&stop_time, NULL);
	gotten_time = (size_t) (stop_time.tv_sec - start_time.tv_sec)*1000000;
	gotten_time += (size_t)(stop_time.tv_usec - start_time.tv_usec);
	call_run_time = call_stop - call_start;
	init_run = init_end - init_start;
	printf(" run time = %zd\n",call_run_time);
	printf(" sum_run time = %zd\n",sum_run_time);
	printf(" gettimeofday_run time = %zd\n",gotten_time);
	printf(" init_run = %zd, init_end = %zd, call_start = %zd\n",init_run, init_end, call_start);
	printf("total = %lu\n",total);

//  printout
	printf(" average cycles per iteration = %f\n", (double)call_run_time/((double)PRIME_VAL*mult));
	return main_ret;
}
