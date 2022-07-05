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

static inline unsigned long long _rdtsc(void)
{
        DECLARE_ARGS(val, low, high);

        asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));

        return EAX_EDX_VAL(val, low, high);
}

u64 br_kernel(int* list, int br_count)
{
	int i,j,k=0;
	u64 isum=0;
	size_t start, stop, run_time;
	j = 1;
        start = _rdtsc();
        for( i=0; i< br_count; i++){
                if(list[k] == 0){
//                      sum += sqrt((val1 + (double)i)/(val1));
//                      sum += (double) j;
                        isum += j;
                }else{
//                      sum += sqrt((val1 - (double)i)/(val1));
//                      sum -= (double) j;
                        isum -= j;
                }
                k++;
                if(k >= MAX_RAN-1) k=0;
        }
        stop = _rdtsc();
        run_time = stop - start;
//      fprintf(stderr," runtime = %zd, sum = %f6.4\n",run_time,sum);
        fprintf(stderr," runtime = %zd, isum = %zd\n",run_time,isum);
	return isum;
}
