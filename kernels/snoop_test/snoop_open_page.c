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
uint64_t read_sum_tsc=0, total_lines;
__pid_t pid=0;
int cpu, cpu_read, cpu_write;

extern size_t * read_buf(int len, size_t * buf);
extern size_t * write_buf(int len, size_t * buf);

static inline unsigned long long _rdtsc(void)
{
        DECLARE_ARGS(val, low, high);

        asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));

        return EAX_EDX_VAL(val, low, high);
}

double drand48(void);

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

void * driver0(void * arg)
{
	int i,j,k, iter_count =0;
	uint64_t line_count=0, init_tsc, end_tsc;
	size_t * read_pntr;

	read_pntr = array;
// pin core affinity
	if(pin_cpu(pid, cpu_read) == -1) {
		err(1,"cannot set cpu read affinity");
		}
	else{
		printf(" read thread pinned to core %d to run\n",cpu_read);
		}
	fprintf(stderr,"total_lines = %ld\n",total_lines);

	read_sum_tsc = 0;
	while(line_count < total_lines)
		{
		i = 0;
		while(exchange_flag == 0)
			{
			i++;
			}
		init_tsc = _rdtsc();
//		if(iter_count < 10)fprintf(stderr,"reader calling kernel\n");
		read_pntr = read_buf(seg_size, read_pntr);
//		if(iter_count < 10)fprintf(stderr,"reader returned from kernel\n");
		end_tsc = _rdtsc();
		read_sum_tsc += (end_tsc - init_tsc);
		line_count += seg_size;
		iter_count++;
		exchange_flag = 0;
		}
	fprintf(stderr," from read thread, line_count = %ld, TSC sum = %lu, latency = %g\n",
			line_count, read_sum_tsc,(double)read_sum_tsc/(double)line_count);
	pthread_exit(NULL);
}

void * driver1(void* arg)
{
	int i,j,k, iter_count = 0;
	uint64_t line_count=0;
	size_t * write_pntr;
	write_pntr = array;
// pin core affinity
	if(pin_cpu(pid, cpu_write) == -1) {
		err(1,"cannot set cpu write affinity");
		}
	else{
		printf(" write thread pinned to core %d to run\n",cpu_write);
		}
	fprintf(stderr,"from writer, total_lines = %ld, shared = %d\n",
		total_lines,shared);
	
	while(line_count < total_lines)
		{
		i = 0;
		while(exchange_flag == 1)
			{
			i++;
			}
		if(shared == 0)
			{
//			if(iter_count < 10)fprintf(stderr,"writer calling write kernel\n");
			write_pntr = write_buf(seg_size, write_pntr);
//			if(iter_count < 10)fprintf(stderr,"writer returned from write kernel\n");
			}
		else 
			{
//			if(iter_count < 10)fprintf(stderr,"writer calling read kernel\n");
			write_pntr = read_buf(seg_size, write_pntr);
//			if(iter_count < 10)fprintf(stderr,"writer returned from read kernel\n");
			}
		line_count += seg_size;
		iter_count++;
		exchange_flag = 1;
		}
	pthread_exit(NULL);
	
}

void usage()
	{
	fprintf(stderr," rndm_walker requires 6 arguments and has a seventh optional argument\n");
	fprintf(stderr," -iN  -i signifies intialization N indicates the core on which the buffer should be initialized\n");
	fprintf(stderr," -rM  -r signifies read M indicates which core the reader thread should be executed on\n");
	fprintf(stderr," -wM  -w signifies \"writer\" indicates which core the \"writer\" thread should be executed on\n");
	fprintf(stderr,"      the -s option will change this thread to only read the lines without modifying them\n");
	fprintf(stderr," -LN  -L signifies lines, N indicates the number of lines used in the full buffer\n");
	fprintf(stderr,"     thus the size controls in which level of cache/memory the full buffer will reside\n");
	fprintf(stderr," -ln  -l signifies lines, n indicates the number of lines used in the pointer chase loop\n");
	fprintf(stderr,"      this size controls in which level of cache/memory the partial buffer, which is exchanged between the threads, will reside\n");
	fprintf(stderr," -SN  -S signifies segments N indicates the number of segments that the buffer will be divided into.\n");
	fprintf(stderr,"         The number of segments must be greater than the number of memory access streams the HW prefetcher can track for the randomization to defeat the HW prefetcher.\n");
	fprintf(stderr,"          if it is set to 1 then the linked list will walk through the buffer in order. \n");
	fprintf(stderr,"          This will yield an open page latency if the HW prefetchers are completely disabled and the buffer is much larger than the LLC.\n");
	fprintf(stderr,"          if it is set to a reasonably large value (32/64)  then the linked list will walk through the segments of the buffer in randomized order.\n");
	fprintf(stderr,"          This will yield a closed dram latency if the HW prefetchers are enabled or disabled and the buffer is much larger than the LLC.\n");
	fprintf(stderr,"          The line count must be a multiple of this value\n");
	fprintf(stderr," -mN  -m signifies multiplier N indicates the multiplying factor applied to the outer loop tripcount. \n");
	fprintf(stderr,"           By default a total of 1.024 Billion iterations of the cacheline walk are executed. N >= 1\n");
	fprintf(stderr," -s    indicates that the lines accessed as \"shared\" and thus the w thread will not modify the line\n");
	}

int main(int argc, char ** argv)
{
	char * buf1;
	void * ret;
	size_t ret_val = 0;
	size_t  array_stride;
	int rc0, rc1;
	int i,j,k, line_count=0,stride=0, fd = -1;
	off_t offset = 0;
	int len=10240000, iter=10,mult=1,main_ret=0;
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

	if(argc < 6){
		fprintf(stderr,"the random walker requires at least 6 arguments (only the 7th in the list below is optional), there were %d\n",argc);
		usage();
		err(1,"insufficient invocation arguments");
		}

	while ((c = getopt(argc, argv, "i:r:w:l:S:m:L:sh")) != -1) {
		switch(c) {
		case 'i':
			cpu = atoi(optarg);
			break;
		case 'r':
			cpu_read = atoi(optarg);
			break;
		case 'w':
			cpu_write = atoi(optarg);
			break;
		case 'l':
			seg_size = atoi(optarg);
			break;
		case 's':
			shared = 1;
			break;
		case 'S':
			num_seg = atoi(optarg);
			break;
		case 'm':
			mult = atoi(optarg);
			break;
		case 'L':
			line_count = atoi(optarg);
			break;
		case 'h':
			usage();
			exit(1);
		default:
			err(1, "unknown option %c", c);
		}
	}
	iter = iter*mult;
	total_lines = len*iter;
	fprintf(stderr," seg_size = %d, line_count = %d, total_lines = %ld\n",
		seg_size, line_count, total_lines);

	var_size = sizeof(size_t);
	fprintf(stderr, "size_t in %zd bytes\n",var_size);
	Thread_dat = (pthread_t *) malloc(MAX_THREAD*sizeof(pthread_t));
	if(Thread_dat == NULL)
		err(1, "malloc of Thread failed");

// pin core affinity

	if(pin_cpu(pid, cpu) == -1) {
		err(1,"failed to set affinity");
		}
	else{
		fprintf(stderr," process pinned to core %d\n",cpu);
		}

	pattern = (int*) malloc(num_seg*sizeof(int));
	if(pattern == NULL)
		{
		fprintf(stderr," failed to malloc pattern for size = %d\n",num_seg);
		err(1,"malloc of pattern failed");
		}

// calculate stride and buffer size
	stride = page_size*stride + 64;
	buf_size = (size_t)line_count*(size_t)stride;
	num_pages = buf_size/page_size + 2;
	buf_size = page_size*num_pages;
	array_stride = stride/sizeof(size_t *);
	iterations = (double)iter*(double)len;

//    create index array for "random" patterna
	index = (int*)malloc(line_count*sizeof(int));
	if(index == NULL)
		{
		fprintf(stderr," failed to malloc index array for line_count of %d\n",line_count);
		err(1,"failed to malloc index");
		}
	if(num_seg == 1)
		{
		for(i=0; i<line_count; i++)index[i] = i;
		}
	else
		{
		
//	fprintf(stderr," calling rndm_list, n = %d\n",num_seg);
		rndm_list(pattern,num_seg);
		lc_by_num_seg = line_count/num_seg;
		if(lc_by_num_seg*num_seg != line_count)
			{
			fprintf(stderr," line count must be a multiple of the fifth argument num_seg = %d\n", num_seg);
			err(1," bad line_count");
			}
		count=0;
		buf_by_num_seg = buf_size/num_seg;
		for(i=0; i<lc_by_num_seg; i++)
			{
			step = i*num_seg;
			for(j=0;j<num_seg;j++)
				{
				count++;
				ind = lc_by_num_seg*pattern[j];
				index[count]= (int) pattern[j] + step;
				if(index[count] >= line_count)
					printf(" count = %d, index = %d\n",count,index[count]);
				}
			count++;
			index[count]=(i+1)*num_seg;
			}
		index[count] = 0;
		}
        index[0] = 0;
//      test index map for every value between 0 and line_count-1 showing up once

        for(i=0;i<line_count;i++)
                {
                if(index[i] > line_count)err(1,"index[%d] = %d",i,index[i]);
//              if(i < 128 )fprintf(stderr,"index[%d] = %d\n",i,index[i]);
//              if(i > line_count - 128 )fprintf(stderr,"index[%d] = %d\n",i,index[i]);
                index_test[index[i]]++;
                }

        bad = 0;
        for(i=0;i<line_count;i++)
                {
                if(index_test[i] != 1)
                        {
                        bad++;
                        if(bad < 64)fprintf(stderr,"index_test[%d] = %d\n",i,index_test[i]);
                        }
                }
        fprintf(stderr,"bad = %d\n",bad);


// malloc and initialize buffers

	buf1 = (char *)malloc(buf_size + 4096 );

//    replace malloc call with a call to mmap
/*
	if(huge == 0)
	buf1 = (char*) mmap(NULL,buf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , fd, offset);
	if(huge == 1)
	buf1 = (char*) mmap(NULL,buf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_HUGETLB , fd, offset);
	if(buf1 == MAP_FAILED)
		{
		fprintf(stderr,"mmap failed\n");
		err(1,"mmap failed");
		} 
*/
	fprintf(stderr," buf1 = %p\n",buf1);
//	buf1 = buf1 + (0x1000 - (size_t)buf1 & 0xFFF) ;
//	fprintf(stderr," buf1 = %p\n",buf1);

	zero_loop = buf_size/(size_t)var_size;
	fprintf(stderr, " buf_size = %zu, zero_loop = %zu, array_stride = %zd\n",buf_size,zero_loop,array_stride);
//	for(i=0;i<buf_size;i++)buf1[i]=0;   //touch every page to ensure creation
	array = (size_t *) buf1;
//	for(i=0; i<zero_loop; i++) array[i] = 0;
	ret = memset(buf1, 0, (size_t)buf_size);
	fprintf(stderr," finished zeroing buf ret = %p\n",ret);

//	for(jj=0;jj<line_count-1; jj++)array[jj*(size_t)array_stride] = (size_t) &array[(size_t)array_stride*(jj+1)];
	for(jj=0;jj<line_count-1;jj++)array[index[jj]*array_stride] = (size_t)&array[index[jj+1]*array_stride];
	fprintf(stderr," target of last element in loop = %zx\n",(size_t)(array[line_count-1]-(size_t)buf1));
	array[(size_t)array_stride*index[line_count-1]] = (size_t)&array[0];

//	for(jj=0; jj< line_count; jj+=8) printf(", jj = %d, array[jj]-&array[0]/array_stride = %d\n",jj,(array[jj]-(size_t)&array[0])/array_stride);

// run the walker
	printf(" invoking reader %d times which loops  %d times on buffer of %d lines with a stride of %d, for a total size of %zu\n",iter,len,line_count,stride,buf_size);

	exchange_flag = 0;
	rc0 = pthread_create(&Thread_dat[0], NULL, driver0, (void *)arg);
	if(rc0)
		err(1,"failed to start thread for driver0");

	printf(" invoking writer %d times which loops  %d times on buffer of %d lines with a stride of %d, for a total size of %zu\n",iter,len,line_count,stride,buf_size);
	rc1 = pthread_create(&Thread_dat[1], NULL, driver1, (void *)arg);
	if(rc1)
		err(1,"failed to start thread for driver1");
	printf(" run time = %zd\n",call_run_time);

//  printout
	printf(" average cycles per iteration = %f\n", (double)call_run_time/iterations);
	pthread_exit(NULL);
	return main_ret;
}
