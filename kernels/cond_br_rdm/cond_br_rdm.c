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
#include <sys/time.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stdlib.h>
#include <err.h>
#include <sched.h>
#include <getopt.h>
#include <math.h>
//#include<linux/ktime.h>

#define MAX_RAN 10007

typedef unsigned long long u64;
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high)     ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)

double drand48(void);
struct timespec current_kernel_time(void);

extern u64 br_kernel(int* list, int br_count);

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

void rndm_list(int* list, int n)
{
        int i,j,k, count, count2;
        double val, tmp, sum = 0;

        val = 0.;
        for(i=0;i<100;i++)val+=drand48();
        count = n-1;
        for(i=0; i<n; i++)
                {
                val = drand48();
                j = (int) (0.5 + val);
                list[i] = j;
		sum += (double) list[i];
                }
	fprintf(stderr," sum of list = %f, MAX_RAN = %d\n",sum, MAX_RAN);
//      for(k=0;k<50;k++)printf(" k = %d, list[k] = %d\n",k,list[k]);
//      printf("\n");
}


void usage()
        {
	fprintf(stderr," cond_br_rdm needs 4 arguemnts\n");
	fprintf(stderr," -iN  -i signifies intialization N indicates the core on which the buffer should be initialized\n");
        fprintf(stderr," -rM  -r signifies run M indicates which core the pointer walker should be executed on\n");
        fprintf(stderr," -lN  -l signifies the number of conditional branches the core loop should execute\n");
	fprintf(stderr," -Nm  -N can be 0 or 1, 0 signifying randomize the execution, 1 fixed choice of branch direction\n");
	fprintf(stderr," -Tm  -N can be 0 or 1, 0 signifying the fixed choice of branch direction\n");
	}
int main(int argc, char ** argv)
{
        char * buf1;
        void * ret;
        int step, c, bad;
	int cpu, cpu_run, br_count, rdm,dir=0;
	int list[MAX_RAN], i,j,k;
	double sum=0.0, val1, val2;
	u64 isum=0;
	size_t start, stop, run_time;
        __pid_t pid=0;

	if(argc < 5){
                fprintf(stderr,"the random walker requires at least 6 arguments (only the 7th in the list below is optional), there were %d\n",argc);
                usage();
                err(1,"insufficient invocation arguments");
                }

        while ((c = getopt(argc, argv, "i:r:l:N:T:")) != -1) {
                switch(c) {
                case 'i':
                        cpu = atoi(optarg);
                        break;
                case 'r':
                        cpu_run = atoi(optarg);
                        break;
                case 'l':
                        br_count = atoi(optarg);
                        break;
                case 'N':
                        rdm = atoi(optarg);
                        break;
                case 'T':
                        dir = atoi(optarg);
                        break;
                default:
                        err(1, "unknown option %c", c);
                }
        }
	if((rdm != 0) && (rdm != 1)){
		fprintf(stderr, " rdm = %d, N must be 0 or 1\n", rdm);
		usage();
                err(1,"bad invocation arguments");
                }
	if(rdm == 1){
		if ((dir != 0) && (dir != 1)){
			fprintf(stderr,"N = %d, T = %d, if rdm = 1 then T must be 0 or 1\n",rdm,dir);
			usage();
			err(1,"bad invocation arguments");
		}
	}
// pin core affinity

        if(pin_cpu(pid, cpu) == -1) {
                err(1,"failed to set affinity");
                }
        else{
                fprintf(stderr," process pinned to core %d\n",cpu);
                }

	if(rdm == 0){
		rndm_list( list, MAX_RAN);
	}else{
		for(i=0; i<MAX_RAN; i++)list[i] = dir;
	}

	val1 = (double)MAX_RAN;

	k=0;
// pin core affinity
        if(pin_cpu(pid, cpu_run) == -1) {
                err(1,"cannot set cpu run affinity");
                }
        else{
                printf(" process pinned to core %d to run\n",cpu_run);
                }
/*	j = 1;
	start = _rdtsc();
	for( i=0; i< br_count; i++){
		if(list[k] == 0){
//			sum += sqrt((val1 + (double)i)/(val1));
//			sum += (double) j;
			isum += j;
		}else{
//			sum += sqrt((val1 - (double)i)/(val1));
//			sum -= (double) j;
			isum -= j;
		}
		k++;
		if(k >= MAX_RAN-1) k=0;
	}
	stop = _rdtsc();
	run_time = stop - start;
//	fprintf(stderr," runtime = %zd, sum = %f6.4\n",run_time,sum);
	fprintf(stderr," runtime = %zd, isum = %zd\n",run_time,isum);
*/
        start = _rdtsc();

	isum = br_kernel(list,br_count);
        stop = _rdtsc();
        run_time = stop - start;

	fprintf(stderr,"  runtime = %zd, isum = %zd\n",run_time,isum);
}
