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

//     Generic Optimization Data Analyzer
//     Dispencer of wisdom and Insight
//     aka   DWI

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#include <malloc.h>
#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"
#include "gooda_util.h"

void
qsindex(index_data *data, int left, int right)
{

        int l_old, r_old, piv_index;
        int piv,index;

        l_old = left;
        r_old = right;
        piv = data[left].val;
        index = data[left].index;

        while(left < right)
                {
                while( (data[right].val >= piv) && (left < right))
                        right--;
                if(left != right)
                        {
                        data[left].val = data[right].val;
                        data[left].index = data[right].index;
                        left++;
                        }
                while( ( data[left].val <= piv) && (left < right))
                        left++;
                if(left != right)
                        {
                        data[right].val = data[left].val;
                        data[right].index = data[left].index;
                        right--;
                        }
                }
        data[left].val = piv;
        data[left].index = index;
        piv_index = left;
        left = l_old;
        right = r_old;
        if(left < piv_index)
                qsindex(data, left, piv_index-1);
        if(right > piv_index)
                qsindex(data, piv_index+1, right);
}
void
quickSortIndex(index_data *arr, int elements)
{
        qsindex(arr, 0, elements-1);
}


void
init_order(void)
{
	int arch_val;
	if((family == 6) && (model == 44))
		{
//		WSM-EP
		arch_val = 1;
		init_order_intel(arch_val);
		return;
		}
	else if ((family == 6) && (model == 42))
		{
//		SNB Desktop
		arch_val = 4;
		init_order_intel(arch_val);
		return;
		}
	else if ((family == 6) && (model == 45))
		{
//		SNB-EP
		arch_val = 5;
		init_order_intel(arch_val);
		return;
		}
	else if ((family == 6) && (model == 58))
		{
//		IVB
		arch_val = 6;
		init_order_intel(arch_val);
		return;
		}
	else if ((family == 6) && (model == 62))
		{
//		IVB_EP
		arch_val = 7;
		init_order_intel(arch_val);
		return;
		}
	else if ((family == 6) && (model == 60))
		{
//		HSW
		arch_val = 8;
		init_order_intel(arch_val);
		return;
		}
	else if ((family == 6) && (model == 63))
		{
//		HSW_EP
		arch_val = 9;
		init_order_intel(arch_val);
		return;
		}
	else
		{
//		default
		init_order_def();
		return;
		}
}

void 
branch_eval(int* sample_count)
{
	if((family == 6) && (model == 44))
		{
//		WSM-EP
		branch_eval_intel();
		return;
		}
	else if ((family == 6) && (model == 42))
		{
//		SNB Desktop
		branch_eval_intel();
		return;
		}
	else if ((family == 6) && (model == 45))
		{
//		SNB-EP
		branch_eval_intel();
		return;
		}
	else if ((family == 6) && (model == 58))
		{
//		IVB
		branch_eval_intel();
		return;
		}
	else if ((family == 6) && (model == 62))
		{
//		IVB_EP
		branch_eval_intel();
		return;
		}
	else if ((family == 6) && (model == 60))
		{
//		HSW
		branch_eval_intel();
		return;
		}
	else if ((family == 6) && (model == 63))
		{
//		HSW_EP
		branch_eval_intel();
		return;
		}
	else
		{
//		default
		branch_eval_def();
		return;
		}
}

event_order_struc_ptr 
set_order(int* sample_count)
{
	event_order_struc_ptr retval;
	if((family == 6) && (model == 44))
		{
//		WSM-EP
		retval = set_order_intel(sample_count, 1);
		return retval;
		}
	else if ((family == 6) && (model == 42))
		{
//		SNB Desktop
		retval = set_order_intel(sample_count, 4);
		return retval;
		}
	else if ((family == 6) && (model == 45))
		{
//		SNB-EP
		retval = set_order_intel(sample_count, 5);
		return retval;
		}
	else if ((family == 6) && (model == 58))
		{
//		IVB
		retval = set_order_intel(sample_count, 6);
		return retval;
		}
	else if ((family == 6) && (model == 62))
		{
//		IVB_EP
		retval = set_order_intel(sample_count, 7);
		return retval;
		}
	else if ((family == 6) && (model == 60))
		{
//		HSW
		retval = set_order_intel(sample_count, 8);
		return retval;
		}
	else if ((family == 6) && (model == 63))
		{
//		HSW_EP
		retval = set_order_intel(sample_count, 8);
		return retval;
		}
	else
		{
//		default
		retval = set_order_def(sample_count);
		return retval;
		}
}

