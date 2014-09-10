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

//     Analysis of structures for Gooda
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
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include "bfd.h"
#include "libiberty.h"
//#include "demangle.h"
#include <time.h>
#include <limits.h>
#include <malloc.h>
#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"
#include "gooda_util.h"
#include "asm_2_src.h"


int reorder_module(process_struc_ptr this_process);
int reorder_rva(module_struc_ptr this_module, process_struc_ptr this_process);
void function_accumulate(module_struc_ptr this_module, process_struc_ptr this_process);
void * inst_working_set(module_struc_ptr this_module);
void printf_rva(sample_struc_ptr, function_struc_ptr, process_struc_ptr);

int first_module = 0;

char* old_module_path=NULL;
int asm_2_src_status;

#define BAD 0xFFFFFFFFFFFFFFFF
#define DMGL_ANSI	(1 << 1)
#define DMGL_PARAMS	(1 << 0)
typedef struct line_struc{
	uint64_t        address;
	int             sample_count;
	}line_data;

typedef struct linkpairs_struc{
	uint64_t	src_trg;
	int		index;
	}linkpairs_data;

typedef struct branch_type_struc{
	int branch;
	int call;
	int conditional;
	int ret;
	int indirect_call;
	int indirect_jmp;
	}branch_type_data;

static struct branch_type_struc branch_type;

// simple string to hash index
int
hash_index(char* str, int table_size)
{
	int i,j,k,len,index;
	uint64_t sum=0;

	len = strlen(str);
	for(i=0; i<len; i++)sum += i*str[i];
	index = sum % table_size;
	return index;
}

//    hex_to_ll
uint64_t 
hex_to_ll(char* s)
{
        int len, i,j, mult, hex_base = 16;
        uint64_t val, badval = 0xFFFFFFFFFFFFFFFFULL;

        val = 0;
        mult = 1;
        len = strlen(s);
	if(len > 16)
		{
		fprintf(stderr," bad string value for hex_to_ll %s\n",s);
		return badval;
		}

        for(i=0; i<len;i++)
                {
                j = len - i -1;
		if(s[j] == ' ')break;
                if((s[j] >= '0') && (s[j] <= '9')) val += (s[j] - '0')*mult;
                else if((s[j] >= 'a') && (s[j] <= 'f')) val += (s[j] - 'a' + 10)*mult;
                else if((s[j] >= 'A') && (s[j] <= 'F')) val += (s[j] - 'A' + 10)*mult;
		else
			{
			fprintf(stderr," bad string value for hex_to_ll %s\n",s);
			return badval;
			}

                mult *= hex_base;
                }
        return val;
}

void* 
binsearch(addr_list_data* list, int num, uint64_t address)
{
	int mid, lo, hi, i;
	void* ptr_val;
	basic_block_struc_ptr this_block;
//	check to see if address is out of range or in first or last ranges
//	fprintf(stderr," address of list = 0x%"PRIx64", num = %d, address = 0x%"PRIx64"\n",list,num,address);
//	for(i=0; i<num; i++)
//		fprintf(stderr," address of list[%d] = 0x%"PRIx64", base = 0x%"PRIx64", len = 0x%"PRIx64", ptr = 0x%"PRIx64"\n",i,&list[i],list[i].base,list[i].len,list[i].struc_ptr);
//	fprintf(stderr," address = 0x%"PRIx64", list[0].base = 0x%"PRIx64", list[num-1].base = 0x%"PRIx64", list[num-1].len = 0x%"PRIx64"\n",
//		address,list[0].base, list[num-1].base, list[num-1].len);
	if(address > list[num-1].base+list[num-1].len)return NULL;
	if(address >= list[num-1].base)return list[num-1].struc_ptr;
	if(address < list[0].base)return NULL;
	if(address <= list[0].base+list[0].len)return list[0].struc_ptr;
	lo = 0;
	hi = num-1;

//	fprintf(stderr," address = 0x%"PRIx64"\n",address);
//	for(i=0; i< num; i++) fprintf(stderr," base = 0x%"PRIx64", len = 0x%"PRIx64"\n",list[i].base,list[i].len);
	while(lo <= hi )
		{
		mid = lo + (hi-lo)/2;
		if((address >= list[mid].base) && (address < list[mid+1].base))
			{
//	found the right range..check if the address is actually inside
			if(address > list[mid].base+list[mid].len)
				{
//				fprintf(stderr," found mid = %d, but address beyond base+len, base = 0x%"PRIx64", len = 0x%"PRIx64"\n",mid,list[mid].base,list[mid].len);
//				for(i=0; i< num; i++) fprintf(stderr," base = 0x%"PRIx64", len = 0x%"PRIx64"\n",list[i].base,list[i].len);
				return NULL;
				}
			ptr_val = list[mid].struc_ptr;
			this_block = (basic_block_struc_ptr)ptr_val;
//			fprintf(stderr," returning ptr for index = %d, block = %d\n",mid, this_block->block_count);
			return list[mid].struc_ptr;
			}
		else if (address < list[mid].base)
			{
			hi = mid - 1;
			}
		else {
			lo = mid + 1;
			}
		}
//	you cannot get here
	fprintf(stderr,"fell out of the bottom of binsearch\n");
	err(1,"binsearch failed");
	return NULL;
}

function_struc_ptr
module_binsearch(uint64_t address, functionlist_struc_ptr func_list)
{
	int mid, lo, hi, i, num;
	function_loc_data* list;
	function_struc_ptr this_function;
//	check to see if address is out of range or in first or last ranges
	num = func_list->size;
	list = func_list->list;
#ifdef DBUG
	fprintf(stderr," address of list struc %p, num = %d, address = 0x%"PRIx64"\n",func_list,num,address);
//	for(i=0; i<num; i++)
//		fprintf(stderr," address of list[%d] = 0x%"PRIx64", base = 0x%"PRIx64", len = 0x%"PRIx64", ptr = %p\n",
//			i,&list[i],list[i].base,list[i].len,list[i].this_function);
#endif
	if(address > (uint64_t)(list[num-1].base+(uint64_t)list[num-1].len) )return NULL;
	if(address >= (uint64_t)list[num-1].base)return list[num-1].this_function;
	if(address < (uint64_t)list[0].base)return NULL;
	if(address <= (uint64_t)(list[0].base+(uint64_t)list[0].len) )return list[0].this_function;
	lo = 0;
	hi = num-1;

#ifdef DBUG
	fprintf(stderr," address = 0x%"PRIx64"\n",address);
//	for(i=0; i< num; i++) fprintf(stderr," base = 0x%"PRIx64", len = 0x%"PRIx64"\n",list[i].base,list[i].len);
#endif
	while(lo <= hi )
		{
		mid = lo + (hi-lo)/2;
		if((address >= list[mid].base) && (address < list[mid+1].base))
			{
//	found the right range..check if the address is actually inside
			if(address > list[mid].base+(uint64_t)list[mid].len)
				{
#ifdef DBUG
				fprintf(stderr," found mid = %d, but address beyond base+len, base = 0x%"PRIx64", len = 0x%x\n",mid,list[mid].base,list[mid].len);
//				for(i=0; i< num; i++) fprintf(stderr," base = 0x%"PRIx64", len = 0x%"PRIx64"\n",list[i].base,list[i].len);
#endif
				return NULL;
				}
			this_function = list[mid].this_function;
#ifdef DBUG
			fprintf(stderr," returning ptr for index = %d, function = %s\n",mid, this_function->function_name);
#endif
			return list[mid].this_function;
			}
		else if (address < list[mid].base)
			{
			hi = mid - 1;
			}
		else {
			lo = mid + 1;
			}
		}
//	you cannot get here
	fprintf(stderr,"fell out of the bottom of module_binsearch\n");
//	err(1,"module_binsearch failed");
	return NULL;
}

void 
qs64(uint64_t *data, int left, int right) 
{

	int l_old, r_old, piv_index;
	uint64_t piv;

        l_old = left;
        r_old = right;
        piv = data[left];

        while(left < right)
                {
                while( (data[right] >= piv) && (left < right))
                        right--;
                if(left != right)
                        {
                        data[left] = data[right];
                        left++;
                        }
                while( ( data[left] <= piv) && (left < right))
                        left++;
                if(left != right)
                        {
                        data[right] = data[left];
                        right--;
                        }
                }
        data[left] = piv;
        piv_index = left;
        left = l_old;
        right = r_old;
        if(left < piv_index)
                qs64(data, left, piv_index-1);
        if(right > piv_index)
                qs64(data, piv_index+1, right);
}
void
quickSort64(uint64_t *arr, int elements)
{
	qs64(arr, 0, elements-1);
}

void 
qs2(pointer_data *data, int left, int right) 
{

	int l_old, r_old, piv_index;
	uint64_t piv;
	sample_struc_ptr ptr;

        l_old = left;
        r_old = right;
        piv = data[left].val;
	ptr = data[left].ptr;
        while(left < right)
                {
                while( (data[right].val >= piv) && (left < right))
                        right--;
                if(left != right)
                        {
                        data[left].val = data[right].val;
                        data[left].ptr = data[right].ptr;
                        left++;
                        }
                while( ( data[left].val <= piv) && (left < right))
                        left++;
                if(left != right)
                        {
                        data[right].val = data[left].val;
                        data[right].ptr = data[left].ptr;
                        right--;
                        }
                }
        data[left].val = piv;
        data[left].ptr = ptr;
        piv_index = left;
        left = l_old;
        right = r_old;
        if(left < piv_index)
                qs2(data, left, piv_index-1);
        if(right > piv_index)
                qs2(data, piv_index+1, right);
}

void
quickSort2(pointer_data *arr, int elements)
{
	qs2( arr, 0, elements-1);
}

void 
qsloc(function_loc_data *data, int left, int right) 
{

	int l_old, r_old, piv_index;
	uint64_t piv;
	char *name, *bind;
	uint32_t len;

        l_old = left;
        r_old = right;
        piv = data[left].base;
	len = data[left].len;
	name = data[left].name;
	bind = data[left].bind;
        while(left < right)
                {
                while( (data[right].base >= piv) && (left < right))
                        right--;
                if(left != right)
                        {
                        data[left].base = data[right].base;
                        data[left].len = data[right].len;
                        data[left].name = data[right].name;
                        data[left].bind = data[right].bind;
                        left++;
                        }
                while( ( data[left].base <= piv) && (left < right))
                        left++;
                if(left != right)
                        {
                        data[right].base = data[left].base;
                        data[right].len = data[left].len;
                        data[right].name = data[left].name;
                        data[right].bind = data[left].bind;
                        right--;
                        }
                }
        data[left].base = piv;
        data[left].len = len;
        data[left].name = name;
        data[left].bind = bind;
        piv_index = left;
        left = l_old;
        right = r_old;
        if(left < piv_index)
                qsloc(data, left, piv_index-1);
        if(right > piv_index)
                qsloc(data, piv_index+1, right);
}

void
quickSort_loc(function_loc_data *arr, int elements)
{
	qsloc( arr, 0, elements-1);
}

void 
qs3(line_data *data, int left, int right) 
{

	int l_old, r_old, piv_index;
	int piv;
	uint64_t address;

        l_old = left;
        r_old = right;
        piv = data[left].sample_count;
	address = data[left].address;
        while(left < right)
                {
                while( (data[right].sample_count >= piv) && (left < right))
                        right--;
                if(left != right)
                        {
                        data[left].sample_count = data[right].sample_count;
                        data[left].address = data[right].address;
                        left++;
                        }
                while( ( data[left].sample_count <= piv) && (left < right))
                        left++;
                if(left != right)
                        {
                        data[right].sample_count = data[left].sample_count;
                        data[right].address = data[left].address;
                        right--;
                        }
                }
        data[left].sample_count = piv;
        data[left].address = address;
        piv_index = left;
        left = l_old;
        right = r_old;
        if(left < piv_index)
                qs3(data, left, piv_index-1);
        if(right > piv_index)
                qs3(data, piv_index+1, right);
}

void
quickSort3(line_data *arr, int elements)
{
	qs3( arr, 0, elements-1);
}

void 
qslink(linkpairs_data *data, int left, int right) 
{

	int l_old, r_old, piv_index;
	uint64_t piv;
	int index;

        l_old = left;
        r_old = right;
        piv = data[left].src_trg;
	index = data[left].index;
        while(left < right)
                {
                while( (data[right].src_trg >= piv) && (left < right))
                        right--;
                if(left != right)
                        {
                        data[left].src_trg = data[right].src_trg;
                        data[left].index = data[right].index;
                        left++;
                        }
                while( ( data[left].src_trg <= piv) && (left < right))
                        left++;
                if(left != right)
                        {
                        data[right].src_trg = data[left].src_trg;
                        data[right].index = data[left].index;
                        right--;
                        }
                }
        data[left].src_trg = piv;
        data[left].index = index;
        piv_index = left;
        left = l_old;
        right = r_old;
        if(left < piv_index)
                qslink(data, left, piv_index-1);
        if(right > piv_index)
                qslink(data, piv_index+1, right);
}

void
quickSort_link(linkpairs_data *arr, int elements)
{
	qslink( arr, 0, elements-1);
}

void
printf_rva(sample_struc_ptr this_rva, function_struc_ptr this_function, process_struc_ptr this_process)
{
	int i,j,k;
	fprintf(stderr,"printf_RVA address = 0x%"PRIx64", total_sample_count = %d, function  %s, module %s \n",
		 this_rva->rva, this_rva->total_sample_count, this_function->function_name, this_process->name);
	fprintf(stderr,"Sample array  ");
	for(i=0;i<num_events;i++)fprintf(stderr," %d,",this_rva->sample_count[num_events*(num_cores + num_sockets) + i]);
	fprintf(stderr,"\n");
}

void * 
process_table(void)
{
	process_struc_ptr loop_process;
	module_struc_ptr loop_module;
	int i,j,k;
	char filename[]="./spreadsheets/process.csv";
	FILE * list;
	char mode[] = "w+";
	int num_col, num_col2, process_count;
	event_order_struc_ptr this_event_order;

#ifdef DBUG
	fprintf(stderr,"entering process_table\n");
#endif

//	this_event_order = set_order(global_sample_count);
#ifdef DBUG
	fprintf(stderr,"returned from set_order, fixed = %d, ordered = %d\n",global_event_order->num_fixed, global_event_order->num_ordered);
#endif
	this_event_order = global_event_order;
	if(this_event_order == NULL)
		{
		fprintf(stderr,"global_event_order returned NULL\n");
		err(1,"global_event_order_failed");
		}


	num_col = global_event_order->num_fixed + global_event_order->num_ordered;
	fprintf(stderr,"process:table num_col = %d, fixed = %d, ordered = %d\n",
		num_col, global_event_order->num_fixed, global_event_order->num_ordered);
	list = fopen(filename,mode);
	if(list == NULL)
		{
		fprintf(stderr,"process_table failed to open file %s\n",filename);
		err(1,"failed to open asm listing file");
		}
	fprintf(list,"[\n");
	fprintf(list,"[, \"Process Path\", \"Module Path\",");
	for(k=0; k < num_col; k++)fprintf(list," \"%s\",",global_event_order->order[k].name);
	fprintf(list," ],\n");
	fprintf(list,"[,");
	fprintf(list," \"0:0\", \"1:0\",");
	for(k=0;k<num_col;k++)fprintf(list," \"%d%s\",",2+global_event_order->order[k].base_col,global_event_order->order[k].ctrl_string);
	fprintf(list," ],\n");
	fprintf(list,"[, , \"MSR Programming\",");
	for(k=0; k < num_col; k++)fprintf(list," 0x%"PRIx64",",global_event_order->order[k].config);
	fprintf(list," ],\n");
	fprintf(list,"[, , \"Periods\",");
	for(k=0; k < num_col; k++)fprintf(list," %ld,",global_event_order->order[k].Period);
	fprintf(list," ],\n");
	fprintf(list,"[, , \"Multiplex\",");
	for(k=0; k < num_col; k++)fprintf(list," %5.4lf,",global_event_order->order[k].multiplex);
	fprintf(list," ],\n");
	fprintf(list,"[, , \"Penalty\",");
	for(k=0; k < num_col; k++)fprintf(list," %d,",global_event_order->order[k].penalty);
	fprintf(list," ],\n");
	fprintf(list,"[, , \"Cycles\",");
	for(k=0; k < num_col; k++)fprintf(list," %d,",global_event_order->order[k].cycle);
	fprintf(list," ],\n");
//	the data
	loop_process = principal_process_stack;
	process_count = 1;
	while(loop_process != NULL)
		{
		if(loop_process->total_sample_count <= 0)break;

		fprintf(list,"[, \"%s\", ,",loop_process->name);
#ifdef DBUG
		fprintf(stderr," calling branch_eval for process %s\n",loop_process->name);
#endif
		branch_eval(loop_process->sample_count);
#ifdef DBUG
		fprintf(stderr,"num_col = %d\n",num_col);
		for(j=0; j<num_col; j++)fprintf(stderr," %d,", global_event_order->order[j].index - num_events*(num_cores+num_sockets) );
		fprintf(stderr,"\n");
#endif
//		for(j=0; j<num_col; j++)fprintf(list," %d,",loop_process->sample_count[num_events*(num_cores + num_sockets) + global_event_order->order[j].index ]);
		for(j=0; j<num_col; j++)fprintf(list," %d,",loop_process->sample_count[ global_event_order->order[j].index ]);
		fprintf(list," ],\n");
#ifdef DBUG
		if(process_count == 1)
			{
			fprintf(stderr,"process table global_event_order data, num_events = %d, num_cores = %d, num_sockets = %d\n", num_events, num_cores, num_sockets);
			for(k=0; k<num_events;k++)
				{
				fprintf(stderr,"name = %s, total = %d,",event_list[k].name,loop_process->sample_count[num_events*(num_cores+num_sockets) + k ] );
				for(j=0; j<num_cores; j++)
					fprintf(stderr," %d,",loop_process->sample_count[num_cores*k +j ]);
				fprintf(stderr,"\n");
				}
			for(j=0; j<num_col; j++)fprintf(stderr," name = %s, index = %d, %d\n",global_event_order->order[j].name,global_event_order->order[j].index,loop_process->sample_count[ global_event_order->order[j].index ]);
			}
#endif

#ifdef DBUG
		fprintf(stderr," %s has %d events\n",loop_process->name,loop_process->total_sample_count);
#endif
		loop_module = loop_process->first_module;
		while(loop_module != NULL)
			{
			if(loop_module->total_sample_count <= 0)break;
//		create instruction working set data files if only 1 event collected
//		and this is the first process
			if((num_events == 1) && (process_count == 1))
				{
				if(
				(strcasecmp(event_list[0].name,"instruction_retired") == 0)  ||
				(strcasecmp(event_list[0].name,"inst_retired:prec_dist") == 0) 
					)inst_working_set(loop_module);
				}

			fprintf(list,"[, , \"%s\",",loop_module->path);
			branch_eval(loop_module->sample_count);
			for(j=0; j<num_col; j++)fprintf(list," %d,",loop_module->sample_count[ global_event_order->order[j].index ]);
			fprintf(list," ],\n");
#ifdef DBUG
			fprintf(stderr," %s has %d events\n",loop_module->path,loop_module->total_sample_count);
#endif
			loop_module = loop_module->next;
			}
		loop_process = loop_process->principal_next;
		process_count++;
		}
	fprintf(list,"[, \"Global sample breakdown\", \"all process/modules\",");
//	this has already been called in hotspot_function
//	branch_eval(global_sample_count);
	for(j=0; j<num_col; j++)fprintf(list," %d,",global_sample_count[ global_event_order->order[j].index ]);
	fprintf(list," ]\n");
	fprintf(list,"]\n");
	fclose(list);
//	err(1,"finished process_table");
}

void * 
reorder_process(void)
{
	process_struc_ptr loop_process, this_process, this_process_next, active_stack = NULL;
	module_struc_ptr loop_module;
	int num_process_with_data = 0, module_sample_sum = 0;
	int i,j,k,num_col;

#ifdef DBUG
	fprintf(stderr,"arrived in reorder_process\n");
#endif

	loop_process = principal_process_stack;
	while(loop_process != NULL)
		{
		if(loop_process->total_sample_count > 0)num_process_with_data++;
		loop_process = loop_process->principal_next;
		}
	process_list = (pointer_data *)	malloc(num_process_with_data*sizeof(pointer_data));
#ifdef DBUG
	fprintf(stderr,"malloc done in reorder_process\n");
#endif
	if(process_list == NULL)
		{
		fprintf(stderr," failed to malloc process list \n");
		err(1,"failed to create process list");
		}
	loop_process = principal_process_stack;
	i=0;
	while(loop_process != NULL)
		{
#ifdef DBUG
		fprintf(stderr," process pid = %d, name = %s, count = %d\n",loop_process->pid, loop_process->name,loop_process->total_sample_count);
#endif
		if(loop_process->total_sample_count > 0)
			{
			process_list[i].ptr = (sample_struc_ptr) loop_process;
			process_list[i].val = (uint64_t) loop_process->total_sample_count;
			i++;
#ifdef DBUG
			fprintf(stderr," process pid = %d, name = %s\n",loop_process->pid, loop_process->name);
#endif
			module_sample_sum = reorder_module(loop_process);
#ifdef DBUG
			fprintf(stderr,"calling reorder_module in reorder_process, pid = %d, process_name = %s, samples = %d, module _sum = %d\n",
				loop_process->pid,loop_process->name,loop_process->total_sample_count, module_sample_sum);
#endif	
			if(i > num_process_with_data)
				{
				fprintf(stderr,"analyzer can't count proc's with data!, i = %d, num_proc = %d\n",i,num_process_with_data);
				err(1," analyzer messed up proc with data");
				}
			}
		loop_process = loop_process->principal_next;
		}
#ifdef DBUG
	fprintf(stderr,"about to call quicksort in reorder_process\n");
#endif
	quickSort2(process_list, num_process_with_data);
#ifdef DBUG
	fprintf(stderr,"back from quicksort in reorder_process\n");
#endif

	loop_process = principal_process_stack;
	active_stack = NULL;
	while(loop_process != NULL)
		{
		if(loop_process->total_sample_count == 0)
			{
//	this reverses the order of processes with no samples in the stack
			this_process_next = loop_process->principal_next;
			loop_process->principal_next = active_stack;
			if(active_stack != NULL)active_stack->principal_previous = loop_process;
			active_stack = loop_process;
			loop_process = this_process_next;
			}
		else
			{
			loop_process = loop_process->principal_next;
			}
		}
	for(i=0; i<num_process_with_data; i++)
		{
//	add these to the stack, they are ordered by increasing sample counts
		this_process = (process_struc_ptr)process_list[i].ptr;
		this_process->principal_next = active_stack;
		if(active_stack != NULL)active_stack->principal_previous = this_process;
		active_stack = this_process;
		}
	principal_process_stack = active_stack;

#ifdef DBUG
	fprintf(stderr,"Data for sorted principal processes\n");
	this_process = principal_process_stack;
	while(this_process != NULL)
		{
		fprintf(stderr,"process %s has %d samples\n",this_process->name,this_process->total_sample_count);
		loop_module = this_process->first_module;
		while(loop_module != NULL)
			{
			if(loop_module->total_sample_count == 0)break;
			fprintf(stderr," module path = %s has %d samples\n",loop_module->path,loop_module->total_sample_count);
			loop_module = loop_module->next;
			}
		this_process = this_process->principal_next;
		}
#endif
//	move this to main
//	process_table();

	fprintf(stderr," hottest process = %s, total samples = %d\n",principal_process_stack->name,principal_process_stack->total_sample_count);
	return;
}

int  
reorder_module(process_struc_ptr this_process)
{
	module_struc_ptr loop_module, this_module, this_module_next, active_stack = NULL;
	pointer_data * module_list;
	int num_module_with_data = 0, sample_sum = 0, rva_sample_sum = 0, module_sample_sum=0;
	int i,j,k;

	loop_module = this_process->first_module;
#ifdef DBUG
	if(loop_module == NULL)
		fprintf(stderr," process %s has no modules\n",this_process->name);
#endif
	while(loop_module != NULL)
		{
		if(loop_module->total_sample_count > 0)num_module_with_data++;
#ifdef DBUG
		fprintf(stderr," module %s has %d samples\n",
			loop_module->path,loop_module->total_sample_count);
#endif
		loop_module = loop_module->next;
		}
	module_list = (pointer_data *)	malloc(num_module_with_data*sizeof(pointer_data));
#ifdef DBUG
	fprintf(stderr,"malloc done in reorder_module for %s, num_module_with_data = %d\n",this_process->name,num_module_with_data);
#endif
	if(module_list == NULL)
		{
		fprintf(stderr," failed to malloc module list for process %s, pid = %d\n", this_process->name, this_process->pid);
		err(1,"failed to create module list");
		}
	this_process->module_list = module_list;
	loop_module = this_process->first_module;
	i=0;
	while(loop_module != NULL)
		{
#ifdef DBUG
		fprintf(stderr," module = %s, sample_count = %d\n",loop_module->path, loop_module->total_sample_count);
#endif	
		if(loop_module->total_sample_count > 0)
			{
			sample_sum += loop_module->total_sample_count;
			module_list[i].ptr = (sample_struc_ptr) loop_module;
			module_list[i].val = (uint64_t) loop_module->total_sample_count;
			i++;
//	get function list for modules with samples
#ifdef DBUG
			fprintf(stderr," calling get_functionlist for module = %s\n",loop_module->path);
#endif
//	get a list of address ranges for functions sorted by increasing address
			loop_module->function_list = get_functionlist(loop_module);
#ifdef DBUG
			fprintf(stderr,"calling reorder_rva in reorder_module, module_name = %s, samples = %d\n",
				loop_module->path,loop_module->total_sample_count);
			fprintf(stderr,"call reorder_rva in reorder_module\n");
#endif
//	sort the RVA structures by increasing address
			rva_sample_sum = reorder_rva(loop_module, this_process);
			module_sample_sum += rva_sample_sum;
			if(i > num_module_with_data)
				{
				fprintf(stderr,"analyzer can't count moduless with data!, i = %d, num_modules = %d\n",i,num_module_with_data);
				err(1," analyzer messed up proc with data");
				}
			}
		if(loop_module->function_list != NULL)
			{
			if((aggregate_func_list == 1) || (this_process->pid != pid_ker))
				{
//				exclude psuedo process -1 unless explicitly requested by command option
#ifdef DBUG
		fprintf(stderr,"calling function_accumulate for module = %s, list address = %p\n",loop_module->path,loop_module->function_list);
		fprintf(stderr," module has %d samples and %d rva's\n",rva_sample_sum,loop_module->rva_count);
#endif
//	if there are identified functions and RVA samples construct the function structures for the address ranges with samples
				if((loop_module->function_list->size > 0) && (rva_sample_sum > 0))function_accumulate(loop_module, this_process);
				}
			}
		loop_module = loop_module->next;
		}

#ifdef DBUG
	fprintf(stderr,"analyzer count of modules with data!, i = %d, num_module = %d, sample_sum = %d, proc_sample_count = %d\n",
			i,num_module_with_data, sample_sum, this_process->total_sample_count);
#endif
	if(num_module_with_data == 0) return sample_sum;
#ifdef DBUG
	fprintf(stderr,"call quicksort in reorder_module\n");
#endif
	quickSort2(module_list, num_module_with_data);
#ifdef DBUG
	fprintf(stderr,"back from quicksort in reorder_module\n");
#endif
	loop_module = this_process->first_module;
	active_stack = NULL;
	while(loop_module != NULL)
		{
		if(loop_module->total_sample_count == 0)
			{
//	this reverses the order of modulees with no samples in the stack
			this_module_next = loop_module->next;
			loop_module->next = active_stack;
			if(active_stack != NULL)active_stack->previous = loop_module;
			active_stack = loop_module;
			loop_module = this_module_next;
			}
		else
			{
			loop_module = loop_module->next;
			}
		}
	for(i=0; i<num_module_with_data; i++)
		{
//	add these to the stack, they are ordered by increasing sample counts
		this_module = (module_struc_ptr)module_list[i].ptr;
		this_module->next = active_stack;
		if(active_stack != NULL)active_stack->previous = this_module;
		active_stack = this_module;
		}
	this_process->first_module = active_stack;
	return module_sample_sum;
}

int 
reorder_rva(module_struc_ptr this_module, process_struc_ptr this_process)
{
	sample_struc_ptr this_sample, loop_sample;
	pointer_data * rva_list;
	int rva_count=0, entries,i,j,k,sample_sum, rva_sample_sum;
	int core, event;
	branch_struc_ptr this_call;

#ifdef DBUG
	fprintf(stderr,"arrived in reorder_rva\n");
	fprintf(stderr," module %s  has %d samples, for pid = %d\n",
		this_module->path,this_module->total_sample_count,this_process->pid);
#endif
	if(this_module->this_table == NULL)
		{
		fprintf(stderr," module %s has no table, has %d samples, for pid = %d\n",
			this_module->path,this_module->total_sample_count,this_process->pid);
		err(1," no table in reorder rva");
		}
	entries = this_module->this_table->entries;
#ifdef DBUG
	fprintf(stderr,"entries set in reorder_rva\n");
#endif
//	rva_list = (pointer_data *) malloc(entries*sizeof(sample_data));

	loop_sample = this_module->first_sample;
#ifdef DBUG
	fprintf(stderr,"loop_sample set in reorder_rva\n");
#endif
	if(loop_sample == NULL)
		{
		fprintf(stderr, " screw up in reorder_rva first_sample = nULL, module = %s, samples = %d\n",this_module->path, this_module->total_sample_count);
		err(1,"first_sample == NULL in reorder_rva");
		}
	rva_count = 0;
	sample_sum = 0;
//	debug loop to test we correctly tracted entries/module..to be removed
	while(loop_sample !=NULL)
		{
		rva_count++;
		sample_sum += loop_sample->total_sample_count;
//		if((rva_count %10) == 0)fprintf(stderr," rva_count = %d, sample_count = %d\n",rva_count, sample_sum);
//		if(rva_count > 130)fprintf(stderr," rva_count = %d, sample_count = %d\n",rva_count, sample_sum);


		loop_sample=loop_sample->next;
		}
#ifdef DBUG
	fprintf(stderr," rva count for module %s, rva count = %d, hash_entries = %d\n",this_module->path, rva_count, entries);
	if(rva_count != entries)
		{
		fprintf(stderr,"screwed up rva count for module %s, rva count = %d, hash_entries = %d\n",this_module->path, rva_count, entries);
		}
#endif
	this_module->rva_list = (pointer_data *) malloc(rva_count * sizeof(pointer_data));
	if(this_module->rva_list == NULL)
		{
		fprintf(stderr," failed to malloc rva list for module %s\n", this_module->path);
		err(1,"failed to create rva list");
		}
	loop_sample = this_module->first_sample;
	i=0;
	while(loop_sample !=NULL)
		{
		this_module->rva_list[i].ptr = loop_sample;
		this_module->rva_list[i].val = loop_sample->rva;
		i++;
		loop_sample=loop_sample->next;
		}
		
#ifdef DBUG
	fprintf(stderr,"call quicksort in reorder_rva, i = %d, rva_count = %d\n",i,rva_count);
#endif
//	for(j=0;j<rva_count;j++)fprintf(stderr," ptr = %lp, val = 0x%"PRIx64"\n",this_module->rva_list[j].ptr,this_module->rva_list[j].val);
	quickSort2(this_module->rva_list,rva_count);
// recreate linked list in reverse order so list starts at smallest rva
	this_module->rva_count = rva_count;
	this_module->first_sample = this_module->rva_list[rva_count-1].ptr;
	this_module->first_sample->next = NULL;
#ifdef DBUG
	this_sample = this_module->first_sample;
	this_call = this_sample->call_list;
	if(this_call != NULL)
		fprintf(stderr," this address 0x%"PRIx64" was the return target of 0x%"PRIx64", %d times\n",
				this_sample->rva,this_call->address,this_call->count);
#endif
	if(rva_count > 1)
		{
		for(i=rva_count-2; i>=0; i--)
			{
//			fprintf(stderr," final loop i = %d\n",i);
			this_sample = this_module->rva_list[i].ptr;
			this_sample->next = this_module->first_sample;
			this_module->first_sample->previous = this_sample;
			this_module->first_sample = this_sample;
#ifdef DBUG
//	print out call counts
			this_call = this_sample->call_list;
			while(this_call != NULL)
				{
				fprintf(stderr," this address 0x%"PRIx64" was the return target of 0x%"PRIx64", %d times\n",
					this_sample->rva,this_call->address,this_call->count);
				this_call = this_call->next;
				}
#endif
				
			}
		}
	return sample_sum;
}

void
PPC_elf_fix(char * local_path, function_loc_data * func_data_buffer, int num_func_in_file )
{
	int i,j,k;
	int local_len,ppc_cmd1_len;
	int ppc_line_count,data_line_count,redirect_count;
	FILE *fileppc;
	char line_buf[1024], pntr_field[20],addr_field[20];
	int line_buf_len, buf_len;
	char ppc_cmd1[] = "readelf -x .opd ";
	char *ppc_cmd, *endpntr;
	typedef struct redirect_struc * redirect_struc_ptr;
	typedef struct redirect_struc {
		redirect_struc_ptr	next;
		redirect_struc_ptr	previous;
		uint64_t		pntr;
		uint64_t		address;
		} redirect_data;
	redirect_struc_ptr redirect_stack, this_redirect, redirect_array;
	uint64_t new_base, base_redirect, last_redirect;

	local_len = strlen(local_path);
	ppc_cmd1_len = strlen(ppc_cmd1);
	line_buf_len = strlen(line_buf);

//		ppc_cmd = malloc(ppc_cmd1_len + ppc_cmd2_len + ppc_cmd3_len + local_len + sizeof(uint64_t) + 2);
	ppc_cmd = malloc(ppc_cmd1_len + local_len + 2);
	if(ppc_cmd == NULL)
		err(1, "failed to malloc buffer for ppc_cmd");
	sprintf(ppc_cmd,"%s%s\0",ppc_cmd1,local_path);
#ifdef DBUG
	fprintf(stderr," ppc_cmd = %s\n",ppc_cmd);
#endif
	fileppc = popen(ppc_cmd, "r");
//	fprintf(stderr," returned from popen, fileppc = %ld\n",(uint64_t)fileppc);
	ppc_line_count = 0;
	data_line_count = 0;
	redirect_count = 0;
	redirect_stack = NULL;
	while(fgets(line_buf,line_buf_len,fileppc) != NULL)
		{
		buf_len = strlen(line_buf);
		if(buf_len != 66)continue;
		ppc_line_count++;
#ifdef DBUG
		fprintf(stderr," readelf -x len = %d, line_count = %d, line = %s",buf_len,ppc_line_count,line_buf);
#endif
//			skip first 2 lines
//		if(ppc_line_count < 3)continue;

		data_line_count++;
//			skip every third line as they have no address data
		if(data_line_count == 3)data_line_count = 0;
		if(data_line_count == 0)continue;

		i = 2;
		while(line_buf[i] != ' ')
			{
			pntr_field[i-2] = line_buf[i];
			i++;
			}
		pntr_field[i-1] = '\0';

		i++;
//		decode the data fields, keeping only the first of the triplet which has the address
//		there are 2 data fields per line so there is an alternating pattern of which to keep
		if(data_line_count == 2) i += 18;
		k = 0;
		for(j=0; j<17; j++)
			if(line_buf[i+j] != ' ')
				{
				addr_field[k] = line_buf[i+j];
				k++;
				}
		addr_field[k] = '\0';
//		fprintf(stderr,"pntr_field = %s, addr_field = %s, redirect_count = %d\n",
//			pntr_field, addr_field,redirect_count);
		redirect_count++;
//		malloc structure for linked list to keep pntr_field and addr_field
		this_redirect = (redirect_struc_ptr)malloc(sizeof(redirect_data));
		if(this_redirect == NULL)
			err(1,"malloc of redirect struc failed for struc %d for module %s",redirect_count,local_path);
		this_redirect->address = strtoll(addr_field,&endpntr,16);
		this_redirect->pntr = strtoll(pntr_field,&endpntr,16);
		if(data_line_count == 2)this_redirect->pntr += 8;
		last_redirect = this_redirect->pntr;
		this_redirect->next = NULL;
		this_redirect->previous = NULL;
#ifdef DBUG
		fprintf(stderr,"pntr = 0x%"PRIx64", address = 0x%"PRIx64", redirect_count = %d\n",
			this_redirect->pntr,this_redirect->address,redirect_count);
#endif
		if(redirect_stack == NULL)
			{
			redirect_stack = this_redirect;
			base_redirect = this_redirect->pntr;
			}
		else
			{
			this_redirect->next = redirect_stack;
			redirect_stack->previous = this_redirect;
			redirect_stack = this_redirect;
			}
		}
//	fprintf(stderr," finished while loop in PPC_fix, ppc_line_count = %d, data_line_count = %d, redirect_count = %d\n",
//		 ppc_line_count, data_line_count, redirect_count);
	redirect_array = (redirect_struc_ptr)malloc(redirect_count*sizeof(redirect_data));
	if(redirect_array == NULL)
		err(1,"failed to malloc redirect array of size %d for module %s",redirect_count,local_path);
	i = 0;
	while(redirect_stack != NULL)
		{
		redirect_array[i].address = redirect_stack->address;
		redirect_array[i].pntr = redirect_stack->pntr;
		i++;
		this_redirect = redirect_stack;
		redirect_stack = redirect_stack->next;
		free(this_redirect);
		}

//	fprintf(stderr,"finished loading array and freeing stack, i = %d, base_redirect = 0x%"PRIx64", last redirect = 0x%"PRIx64"\n",
//		i,base_redirect,last_redirect);

	for(i=0; i<num_func_in_file; i++)
		{
		k = func_data_buffer[i].base - base_redirect;
		j = k/24;
		j = redirect_count - j - 1;
		if(j < 0)j = 0;
		if(j > redirect_count-1)j= redirect_count-1;
//		fprintf(stderr," i = %d, j = %d, k = %d, func_data_buffer[i].base = 0x%"PRIx64"\n",
//			i,j,k,func_data_buffer[i].base);
		if(func_data_buffer[i].base > last_redirect)continue;
		new_base = redirect_array[j].address;
		if(redirect_array[j].pntr != func_data_buffer[i].base)
			{
#ifdef DBUG
			fprintf(stderr,"bad redirect for module %s, i = %d base = 0x%"PRIx64", name = %s ",
			local_path,i,func_data_buffer[i].base,func_data_buffer[i].name);
			fprintf(stderr," j = %d, redirect_pntr = 0x%"PRIx64", addr = 0x%"PRIx64"\n",
			j,redirect_array[j].pntr,redirect_array[j].address);
#endif
//			err(1,"bad redirect pntr");
			}
		else
			{

#ifdef DBUG
//			fprintf(stderr," new_base = 0x%lx\n",new_base);
#endif
			func_data_buffer[i].base = new_base;
			}
//	fclose(fileppc);
		}
}

functionlist_struc_ptr 
get_functionlist(module_struc_ptr this_module)
{
	char line_buf[1024], local_bin_dir[] = "./binaries/",  cmd[] = "readelf -s -W ", FUNC[] = "FUNC";
	char machine_cmd[] = "readelf -e ", machine_cmd2[] = " | grep -i ";
	char *module_arch_cmd;
	int module_arch_cmd_len, machine_len, machine_cmd_len=10, machine_cmd2_len=11;
	char *endptr;
	char demangle[] = "c++filt ";
	char field[9][1024], space = ' ', colon = ':';
	char *full_path_cmd, *funcname, *local_name, *local_cmd, *funcname_cmd;
	uint64_t first_ip, previous_func_ip, previous_func_end, new_base;
	function_loc_data * func_data_buffer, *cleaned_func_data_buffer;
	functionlist_struc_ptr this_functionlist;
	function_location_stack_ptr function_loc_stack, this_location;
	uint32_t func_len, previous_func_len, module_len, module_name_len, num_func_in_file, num_func, local_name_len;
	size_t  funcname_len, funcbind_len, local_len = 11, cmd_len = 14, demangle_len = 8;
	int i,j,k, lines_in_file, field_count, char_count;
	FILE *file, *filt, *grep_out;
	char *grep_out_val;
	int access_status;
	int line_buf_len, buf_len, local_flag,free_count, func_count,len_sum = 0;
	int ppc_cmd1_len, ppc_cmd2_len, ppc_cmd3_len;

	num_func_in_file = 0;
	num_func = 0;
	function_loc_stack = NULL;
	local_flag = 0;
	local_len = strlen(local_bin_dir);
	cmd_len = strlen(cmd);
	machine_len = strlen(machine);
	machine_cmd_len = strlen(machine_cmd);
	machine_cmd2_len = strlen(machine_cmd2);
	demangle_len = strlen(demangle);

	if(first_module != 2)first_module = 1;

//	strip off module length and create module name string
	module_len = strlen(this_module->path);
	i = module_len;
	while((this_module->path[i] != '/') && ( i >=0) )i--;
	module_name_len = module_len - i;   // + 1 for \0, -1 to get rid of /
	this_module->module_name = (char*) malloc((module_name_len+1)*sizeof(char));
	if(this_module->module_name == NULL)
		{
		fprintf(stderr," failed to malloc buffer for module name, path = %s\n",this_module->path);
		err(1, "failed to malloc buffer for module name");
		}
	for(j = 0; j< module_name_len; j++) this_module->module_name[j] = this_module->path[i+1+j];
	this_module->module_name[module_name_len] = '\0';
#ifdef DBUG
	fprintf(stderr," module name = %s\n",this_module->module_name);
#endif

	if((strcmp(this_module->module_name,"triad") == 0) && (first_module == 1))
		{
		first_module = 0;
		fprintf(stderr,"this is vmlinux\n");
		}

//	check local bin directory first
	local_name = (char*)malloc(local_len + module_name_len + 1);
	if(local_name == NULL)
		{
		fprintf(stderr," failed to malloc buffer for module local_name, path = %s\n",this_module->path);
		err(1, "failed to malloc buffer for module local_name");
		}
	for(j=0; j< local_len; j++)local_name[j] = local_bin_dir[j];
	for(j=0; j< module_name_len; j++)local_name[j+local_len] = this_module->module_name[j];
	local_name[local_len+module_name_len] = '\0';
#ifdef DBUG
	fprintf(stderr," module local name = %s\n",local_name);
#endif
	access_status = access(local_name, R_OK);
	if(access_status == 0)
		{
		local_flag = 1;
		this_module->local_path = (char*) malloc((local_len+module_name_len + 1)*sizeof(char));
		if(this_module->local_path == NULL)
			{
			fprintf(stderr," failed to malloc buffer for local_path, path = %s\n",this_module->path);
			err(1, "failed to malloc buffer for module local_path");
			}
		module_len = local_len+module_name_len -1;
		for(j=0; j< local_len+module_name_len; j++)this_module->local_path[j] = local_name[j];

		local_cmd = (char*) malloc(cmd_len + local_len + module_name_len + 1);
		if(local_cmd == NULL)
			{
			fprintf(stderr," failed to malloc buffer for local_cmd, path = %s\n",this_module->path);
			err(1, "failed to malloc buffer for module local_cmd");
			}
		for(j=0; j< cmd_len; j++)local_cmd[j] = cmd[j];
		for(j=0; j< local_len + module_name_len; j++)local_cmd[j+cmd_len] = local_name[j];
		local_cmd[cmd_len + local_len + module_name_len] = '\0';
#ifdef DBUG
	fprintf(stderr," module local cmd = %s, val = %p\n",local_cmd, local_cmd);
#endif
		file = popen(local_cmd, "r");
		if(file == NULL)
			{
			fprintf(stderr," file returned 0 for access but readelf -s failed for %s, module path %s\n",local_name,this_module->path);
			err(1, "readelf failed");
			}
		}
	else
		{
//	binary file is not in ./binaries directory so search original path
#ifdef DBUG
	fprintf(stderr," module full name = %s\n",this_module->path);
#endif
		access_status = access(this_module->path, R_OK);
		if(access_status != 0)
			{
#ifdef DBUG
			fprintf(stderr," access could not find module %s\n",this_module->path);
#endif
			return NULL;
			}
		this_module->local_path = (char*) malloc((module_len + 1)*sizeof(char));
		if(this_module->local_path == NULL)
			{
			fprintf(stderr," failed to malloc buffer for local_path to be set to path, path = %s\n",this_module->path);
			err(1, "failed to malloc buffer for module local_path");
			}
		for(j=0; j< module_len; j++)this_module->local_path[j] = this_module->path[j];
		this_module->local_path[module_len] = '\0';
		full_path_cmd = (char*) malloc((module_len+cmd_len+1)*sizeof(char));
		if(full_path_cmd == NULL)
			{
			fprintf(stderr," failed to malloc buffer for full_path_cmd, path = %s\n",this_module->path);
			err(1, "failed to malloc buffer for module full_path__cmd");
			}
		for(j=0; j< cmd_len; j++)full_path_cmd[j] = cmd[j];
		for(j=0; j< module_len; j++)full_path_cmd[j+cmd_len] = this_module->path[j];
		full_path_cmd[module_len+cmd_len] = '\0';
#ifdef DBUG
	fprintf(stderr," module full_path cmd = %s\n",full_path_cmd);
#endif
		file = popen(full_path_cmd, "r");
		if(file == NULL)
			{
			fprintf(stderr," failed to find module for full_path_cmd, path = %s\n",this_module->path);
//			err(1, "failed to find_module for module full_path__cmd");
			return NULL;
			}
		}
//	found module
// 	check architecture
	fprintf(stderr," machine_cmd_len = %d, module_len = %d, machine_cmd2_len = %d, machine_len = %d\n",
		machine_cmd_len, module_len, machine_cmd2_len, machine_len);
	fprintf(stderr," machine_cmd = %s, module = %s, machine_cmd2 = %s, machine = %s\n",
		machine_cmd,this_module->local_path,machine_cmd2,machine);
	module_arch_cmd = (char *)malloc(machine_cmd_len + module_len + machine_cmd2_len + machine_len + 1);
	for(j=0; j< machine_cmd_len; j++)module_arch_cmd[j] = machine_cmd[j];
	for(j=0; j< module_len; j++)module_arch_cmd[machine_cmd_len + j] = this_module->local_path[j];
	for(j=0; j< machine_cmd2_len; j++)module_arch_cmd[machine_cmd_len + module_len + j] = machine_cmd2[j];
	for(j=0; j< machine_len; j++)module_arch_cmd[machine_cmd_len + module_len + machine_cmd2_len + j] = machine[j];
	module_arch_cmd[machine_cmd_len + module_len + machine_cmd2_len + machine_len] = '\0';
	fprintf(stderr," module %s, machine_arch cmd = %s\n", this_module->local_path, module_arch_cmd);
	grep_out = popen(module_arch_cmd, "r");
        if(grep_out == NULL)
                {
                fprintf(stderr," failed to get a pipe for module_arch_cmd, path = %s\n",this_module->path);
                return NULL;
                }
	lines_in_file = 0;
	line_buf_len = 1024;
	grep_out_val = fgets(line_buf,line_buf_len,grep_out);
	if(grep_out_val == NULL)
		{
#ifdef DBUG
		fprintf(stderr," grep_out_val was NULL for %s\n",module_arch_cmd);
#endif
		free(module_arch_cmd);
		return NULL;
		}
		else
		{
#ifdef DBUG
		fprintf(stderr," grep out line_buff was %s\n",line_buf);
#endif
		free(module_arch_cmd);
		}

//	read lines of readelf -s output, skip first 3, remove zero length entries
//	and create a stack
	lines_in_file = 0;
	line_buf_len = 1024;
#ifdef DBUG
		fprintf(stderr," readelf -s output for module %s\n",this_module->path);
#endif
	while(fgets(line_buf,line_buf_len,file) != NULL)
		{
		buf_len = strlen(line_buf);
#ifdef DBUG
		fprintf(stderr," readelf -s len = %d, line = %s",buf_len,line_buf);
#endif
		if(first_module == 0)
			{
			fprintf(stderr," readelf -s len = %d, line = %s",buf_len,line_buf);
			}
//		process the fields
		lines_in_file++;
//		skip first three lines
		if(lines_in_file <= 3) continue;
                field_count = 0;
                char_count = 0;
//  this code limits the function name to around 950 characters
                for(i=0; i< buf_len; i++)
                        {
#ifdef DBUG
				if(field_count >= 9)fprintf(stderr,"field_count too big = %d\n",field_count);
#endif
			if(field_count >= 9)continue;
                        if(line_buf[i] != space)
                                {
				field[field_count][char_count] = line_buf[i];
                                if((field_count == 0) && (line_buf[i] == colon))
                                        field[field_count][char_count] = '\0';
                                if(line_buf[i] == '\n') field[field_count][char_count] = '\0';
                                char_count++;
                                }
                        else
                                {
                                if(char_count !=0)
                                        {
                                        field[field_count][char_count] = '\0';
                                        field_count++;
                                        }
                                char_count = 0;
                                }
                        }
		if(field_count >= 9)continue;
		func_len = atoi(field[2]);
		if(func_len == 0)
			{
			func_len = strtol(field[2], (char **) NULL, 16);
			if(func_len == 0)continue;
			}
		if(strcmp(field[3], FUNC) != 0)continue;
		num_func_in_file++;

		this_location = function_location_stack_create();
		if(this_location == NULL)
			err(1,"failed to create function_location struc in get_functionlist");
		this_location->next = function_loc_stack;
		if(function_loc_stack != NULL) function_loc_stack->previous = this_location;
		function_loc_stack = this_location;
		this_location->base = hex_to_ll(field[1]);
		this_location->base = (this_location->base & addr_mask);
		this_location->len = func_len;

		funcname_len = strlen(field[7]);
		this_location->name = (char*) malloc((funcname_len+1)*sizeof(char));
		if(this_location->name == NULL)
			{
			fprintf(stderr," failed to malloc buffer for function name %s, module %s\n",field[7],this_module->path);
			err(1,"malloc failed for function name");
			}
		for(i=0; i<funcname_len; i++)this_location->name[i] = field[7][i];
		this_location->name[funcname_len] = '\0';

		funcbind_len = strlen(field[4]);
		this_location->bind = (char*) malloc((funcbind_len+1)*sizeof(char));
		if(this_location->bind == NULL)
			{
			fprintf(stderr," failed to malloc buffer for function bind %s, module %s\n",field[4],this_module->path);
			err(1,"malloc failed for function bind");
			}
		for(i=0; i<funcbind_len; i++)this_location->bind[i] = field[4][i];
		this_location->bind[funcbind_len] = '\0';

		for(i=0; i<9; i++)field[i][0] = '\0';
		}
#ifdef DBUG
	fprintf(stderr," lines_in_file = %d, samples in module = %d\n",lines_in_file,this_module->total_sample_count);
#endif
	if((lines_in_file == 0) || (num_func_in_file == 0))
		{
		free(local_name);
		if(local_flag == 1)free(local_cmd);
		if(local_flag == 0)free(full_path_cmd);
		return NULL;
		}
//	create an array from the linked list and sort it by base
	func_data_buffer = (function_loc_data*)malloc(num_func_in_file*sizeof(function_loc_data));
	if(func_data_buffer == NULL)
		{
		fprintf(stderr," failed to malloc buffer for func_data_buffer, module path = %s\n",this_module->path);
		err(1, "failed to malloc buffer for module func_data_buffer");
		}
	this_location = function_loc_stack;
	i = 0;
//	fprintf(stderr," READELF -s -W output for FUNC in %s\n",this_module->local_path);
	while(this_location != NULL)
		{
		func_data_buffer[i].name = this_location->name;	
		func_data_buffer[i].bind = this_location->bind;	
		func_data_buffer[i].base = this_location->base;	
		func_data_buffer[i].len = this_location->len;	
//		fprintf(stderr,"i = %d base = 0x%"PRIx64", len = 0x%"PRIx64", name = %s\n",
//			i,func_data_buffer[i].base,func_data_buffer[i].len,func_data_buffer[i].name);
		i++;
		this_location = this_location->next;
		}

//		fix up of this_location->base needed for PPC64
	if((arch_type_flag == 2) && (this_module->bin_type = 64))
		PPC_elf_fix(this_module->local_path, func_data_buffer,num_func_in_file);

#ifdef DBUG
	fprintf(stderr," calling quicksort_loc for module %s\n",this_module->path);
#endif
	quickSort_loc(func_data_buffer,num_func_in_file);
//	walk through the sorted list and do not copy entries where base < base_prev+len_prev
	cleaned_func_data_buffer = (function_loc_data*)calloc(1, num_func_in_file*sizeof(function_loc_data));
	if(cleaned_func_data_buffer == NULL)
		{
		fprintf(stderr," failed to malloc buffer for cleaned_func_data_buffer, module path = %s\n",this_module->path);
		err(1, "failed to malloc buffer for module cleaned_func_data_buffer");
		}
	j = 0;
	cleaned_func_data_buffer[0].name = func_data_buffer[0].name;
	cleaned_func_data_buffer[0].base = func_data_buffer[0].base;
	cleaned_func_data_buffer[0].len = func_data_buffer[0].len;
	for(i=1; i< num_func_in_file; i++)
		{
		if((cleaned_func_data_buffer[j].base + (uint64_t)cleaned_func_data_buffer[j].len) <= func_data_buffer[i].base)
			{
			j++;
			cleaned_func_data_buffer[j].name = func_data_buffer[i].name;
			cleaned_func_data_buffer[j].bind = func_data_buffer[i].bind;
			cleaned_func_data_buffer[j].base = func_data_buffer[i].base;
			cleaned_func_data_buffer[j].len = func_data_buffer[i].len;
			}
		if((cleaned_func_data_buffer[j].base == func_data_buffer[i].base) &&
			(strlen(func_data_buffer[i].name) < strlen(cleaned_func_data_buffer[j].name)))
			{
			cleaned_func_data_buffer[j].name = func_data_buffer[i].name;
			}
		if(first_module == 0)
			{
			fprintf(stderr," addr = 0x%lx, len = %d, %s,  %s\n",func_data_buffer[i].base,func_data_buffer[i].len,func_data_buffer[i].bind,func_data_buffer[i].name);
			}


/*
		if(strcmp(this_module->module_name,"vmlinux") == 0)
			{
			fprintf(stderr," addr = 0x%lx, len = %d, %s,  %s\n",func_data_buffer[i].base,func_data_buffer[i].len,func_data_buffer[i].bind,func_data_buffer[i].name);
			len_sum += func_data_buffer[i].len;
			}
*/
		}

	this_functionlist = (functionlist_struc_ptr) malloc(sizeof(functionlist_data));
	if(this_functionlist == NULL)
		{
		fprintf(stderr," failed to malloc buffer for this_functionlist, module path = %s\n",this_module->path);
		err(1, "failed to malloc buffer for module this_functionlist");
		}
	this_functionlist->size = j+1;
	this_functionlist->list = cleaned_func_data_buffer;


	if(first_module == 0)
		{
		fprintf(stderr," cleaned Functionlist\n");
		for(i=0; i<j+1; i++)
			{
			fprintf(stderr," addr = 0x%lx, len = %d, %s,  %s\n",
				cleaned_func_data_buffer[i].base,cleaned_func_data_buffer[i].len,
				cleaned_func_data_buffer[i].bind,cleaned_func_data_buffer[i].name);
			}
		first_module=2;
		}
/*
	if(strcmp(this_module->module_name,"triad_inl_g") == 0)
		{
		fprintf(stderr," module = %s, length = %d, summed len = %d\n",this_module->module_name, this_module->length,len_sum);
		}
*/

//	fprintf(stderr," module local name = %s, num_func_in_file = %d\n",local_name, num_func_in_file);
	free(func_data_buffer);
//	fprintf(stderr, "freed func_data_buffer module path = %s\n",this_module->path);
//	fprintf(stderr," module local name = %s, val = %lx\n",local_name, local_name);
#ifdef DBUG
	fprintf(stderr, "freed func_data_buffer module path = %s\n",this_module->path);
	fprintf(stderr," module local name = %s, val = %p\n",local_name, local_name);
#endif
	free(local_name);
//	fprintf(stderr, "freed local_name module path = %s\n",this_module->path);
//	if(local_flag == 1)fprintf(stderr," module local cmd = %s, val = %lx\n",local_cmd, local_cmd);
#ifdef DBUG
	fprintf(stderr, "freed local_name module path = %s\n",this_module->path);
	if(local_flag == 1)fprintf(stderr," module local cmd = %s, val = %p\n",local_cmd, local_cmd);
#endif
	if(local_flag == 1)free(local_cmd);
//	if(local_flag == 1)fprintf(stderr, "freed local_cmd module path = %s\n",this_module->path);
//	fprintf(stderr," module full cmd = %s, val = %lx\n",full_path_cmd, full_path_cmd);
#ifdef DBUG
	fprintf(stderr, "freed local_cmd module path = %s\n",this_module->path);
	if(local_flag == 0)fprintf(stderr," module full cmd = %s, val = %p\n",full_path_cmd, full_path_cmd);
#endif
	if(local_flag == 0)free(full_path_cmd);
//	if(local_flag == 0)fprintf(stderr, "freed full_path_cmd module path = %s, number of functions = %d\n",this_module->path,num_func_in_file);
#ifdef DBUG
	if(local_flag == 0)fprintf(stderr, "freed full_path_cmd module path = %s\n",this_module->path);
#endif
	this_location = function_loc_stack;
	free_count = 0;
	while(this_location != NULL)
		{
		function_loc_stack = this_location->next;
		free(this_location);
		free_count++;
		this_location = function_loc_stack;
		}
//	fprintf(stderr, "freed this_location from stack %d times for module path = %s\n",free_count,this_module->path);
#ifdef DBUG
	fprintf(stderr, "freed this_location from stack %d times for module path = %s\n",free_count,this_module->path);
#endif
	pclose(file);

	return this_functionlist;
}
void
branch_accumulate(func_branch_struc_ptr * this_link, branch_struc_ptr this_branch, 
			sample_struc_ptr this_sample, function_struc_ptr this_function, int srctrg)
{
	func_branch_struc_ptr loop_link;
	branch_site_struc_ptr this_branch_site;

	if(*this_link == NULL)
		{
//		first rva with call_list for this function
		*this_link = func_branch_struc_create();
		if(*this_link == NULL)
			err(1," failed to create func_branch_struc for function %s\n",
					this_function->function_name);
		loop_link = *this_link;
		if(srctrg == 0)this_function->func_sources++;
		if(srctrg == 1)this_function->func_targets++;
		while(this_branch != NULL)
			{
//	copy this_sample's call list to initialize this_function->sources linked list
			loop_link->this_branch_target->address = this_branch->address;
			loop_link->this_branch_target->this_module = this_branch->this_module;
			loop_link->this_branch_target->count = this_branch->count;
#ifdef DBUG
			fprintf(stderr,"branch_accumulate: initializing function %s branch stack with address 0x%"PRIx64", count = %d\n",
				this_function->function_name,loop_link->this_branch_target->address,loop_link->this_branch_target->count);
#endif
			loop_link->branch_site_stack = branch_site_struc_create();
			if(loop_link->branch_site_stack == NULL)
				err(1,"failed to create initial branch_site stack for %s\n",
					this_function->function_name);
			loop_link->branch_site_stack->address = this_sample->rva;
			loop_link->branch_site_stack->count = this_branch->count;
			if(this_branch->next != NULL)
				{
				loop_link->next = func_branch_struc_create();
				if(loop_link->next == NULL)
					err(1,"failed to create next func_branch_struc for %s\n",
						this_function->function_name);
				loop_link = loop_link->next;
				if(srctrg == 0)this_function->func_sources++;
				if(srctrg == 1)this_function->func_targets++;
				}
			this_branch = this_branch->next;
			}
		}
	else
		{
//		this_function already has a source list started
		loop_link = *this_link;
		while(this_branch != NULL)
			{
			while(loop_link->this_branch_target->address != this_branch->address)
//				walk list of fun_struc_ptr this_branch_call_struc's to find call address, create new link as needed
				{
				if(loop_link->next == NULL)
					{
					loop_link->next = func_branch_struc_create();
					if(loop_link->next == NULL)
						err(1,"failed to extend func_branch_struc list for %s\n",
							this_function->function_name);
					loop_link->next->this_branch_target->address = this_branch->address;
					loop_link->next->this_branch_target->this_module = this_branch->this_module;
					if(srctrg == 0)this_function->func_sources++;
					if(srctrg == 1)this_function->func_targets++;
					}
				loop_link = loop_link->next;
				}
//				found the correct target function, increment count and call_site_stack
			loop_link->this_branch_target->count += this_branch->count;
#ifdef DBUG
			fprintf(stderr,"branch_accumulate: updating function %s branch stack with address 0x%"PRIx64", count = %d\n",
				this_function->function_name,loop_link->this_branch_target->address,loop_link->this_branch_target->count);
#endif
//			increment call_site_stack for loop_sample->rva
			this_branch_site = branch_site_struc_create();
			if(this_branch_site == NULL)
				err(1,"failed to extend branch_site stack for %s\n",
					this_function->function_name);
			this_branch_site->next = loop_link->branch_site_stack;
			loop_link->branch_site_stack = this_branch_site;
			loop_link->branch_site_stack->address = this_sample->rva;
			loop_link->branch_site_stack->count = this_branch->count;
			this_branch = this_branch->next;
			loop_link = *this_link;	//reset source stack to top of list
			}
		}

//	fprintf(stderr,"branch_accumulate: *this_link = %p\n",*this_link);
}

void 
function_accumulate(module_struc_ptr this_module, process_struc_ptr this_process)
{

	function_struc_ptr this_function, function_stack, old_function;
	sample_struc_ptr loop_sample, sample_tmp;
	function_loc_data * this_list;
	int i,j,k, event, core, function_count, function_with_data_count, rva_count, old_rva_count, total_rva_count;
	branch_struc_ptr this_branch;
	int derived_start,srctrg;
	loop_sample = this_module->first_sample;
	function_count = this_module->function_list->size;
	this_list = this_module->function_list->list;
	i = 0;
	rva_count = 0;
	total_rva_count = 0;
	old_rva_count = 0;
	this_function = NULL;
	old_function = this_function;
#ifdef DBUG
	fprintf(stderr," module = %s, function_count = %d\n",this_module->path,function_count);
#endif

	while(loop_sample != NULL)
		{
		global_rva++;
//			walk function list until base + len > current RVA
		while((uint64_t)(this_list[i].base +(uint64_t) this_list[i].len) < loop_sample->rva)
		
			{
			if(i >= function_count -1)
				{
				fprintf(stderr," rva is > last function endpoint, rva = 0x%"PRIx64", last base = 0x%"PRIx64", len = 0x%"PRIx64" name = %s, rva_count = %d, module_rva_count = %d, module = %s\n",
				loop_sample->rva,this_list[i].base,(uint64_t)this_list[i].len,this_list[i].name,total_rva_count,this_module->this_table->entries,this_module->path);
//				err(1,"rva beyond function range in function_accumulate");
				return;
				}
			i++;
			this_function = NULL;
			}
//			check that current RVA is not < base...if so RVA is not in a function range
		if(loop_sample->rva < (uint64_t) this_list[i].base)
			{
			bad_rva++;
			bad_sample_count += loop_sample->total_sample_count;
			total_function_sample_count += loop_sample->total_sample_count;
			total_rva_count++;
#ifdef DBUG
			fprintf(stderr,"Offset 0x%"PRIx64", with %d samples, in module %s\n",loop_sample->rva,loop_sample->total_sample_count,this_module->module_name);
#endif
			loop_sample = loop_sample->next;
			continue;
			}
//		found the function that includes this RVA
//		fprintf(stderr," this_function address = 0x%"PRIx64"\n",this_function);
		if(this_function == NULL)
			{
			old_rva_count = rva_count;
			rva_count = 0;
			this_function = function_struc_create();
			if(this_function == NULL)
				{
				fprintf(stderr," failed to create function struc in function allocate for module = %s\n",this_module->path);
				err(1,"failed to create function struc");
				}
			this_list[i].this_function = this_function;
			this_function->function_name = this_list[i].name;
			this_function->function_mangled_name = this_list[i].name;
			this_function->function_length = this_list[i].len;
			this_function->function_rva_start = this_list[i].base;
			this_function->first_rva = loop_sample;
			this_function->this_module = this_module;
			this_function->this_process = this_process;
			this_function->next = global_func_stack;
			if(global_func_stack != NULL)global_func_stack->previous = this_function;
			global_func_stack = this_function;
			global_func_count++;
#ifdef DBUG
			if(old_function != NULL)
			fprintf(stderr," this_function address = %p, name = %s, len = %ld, rva_count = %d, total_rvas = %d, cycle_samples= %d, total_samples = %d\n",
				old_function, old_function->function_name,old_function->function_length, old_rva_count,total_rva_count, old_function->cycle_count, old_function->total_sample_count);
#endif
			}
//		increment rva socket, total and module/process sample_count arrays
		for(event = 0; event < num_events; event++)
			{
			for(core = 0; core < num_cores; core++)
				{
				this_function->cycle_count += loop_sample->sample_count[core];
				if(loop_sample->sample_count[event*num_cores + core] != 0)
					{
					loop_sample->sample_count[num_events*(num_cores + num_sockets) + event] += 
							loop_sample->sample_count[event*num_cores + core];
					this_function->sample_count[num_events*(num_cores + num_sockets) + event] += 
							loop_sample->sample_count[event*num_cores + core];
					this_function->sample_count[event*num_cores + core] += loop_sample->sample_count[event*num_cores + core];
//		per socket accumulation missing at this time due to lack of topology
//					global_sample_count_in_func += loop_sample->sample_count[event*num_cores + core];
					}
				}
			}
#ifdef DBUG
//      follow a single address through if there are worries about lost samples/rva struc's
//		if(loop_sample->rva == 0x43e110)fprintf(stderr,"agggregate function for address 0x43e110, cycle count = %d, function name = %s\n",
//				loop_sample->sample_count[num_events*(num_cores + num_sockets)], this_function->function_name);
//		if(loop_sample->rva == 0x43e111)fprintf(stderr,"agggregate function for address 0x43e111, cycle count = %d, function name = %s\n",
//				loop_sample->sample_count[num_events*(num_cores + num_sockets)],this_function->function_name);
#endif
//		aggregate the derived events
		if(source_index != 0)
			this_function->sample_count[source_index] += loop_sample->sample_count[source_index];
		if(target_index != 0)
			this_function->sample_count[target_index] += loop_sample->sample_count[target_index];
		if(next_taken_index != 0)
			this_function->sample_count[next_taken_index] += loop_sample->sample_count[next_taken_index];

//			aggregate call_list into functions sources list
		this_branch = loop_sample->return_list;
		if(this_branch != NULL)
			{
#ifdef DBUG
			fprintf(stderr,"return_list non zero for function %s, at address 0x%"PRIx64"\n",
				this_function->function_name,loop_sample->rva);
			fprintf(stderr," return_list count = %d from address 0x%"PRIx64"\n",this_branch->count, this_branch->address);
			fprintf(stderr,"function_accumulate: &this_function->sources = %p\n",&this_function->sources);
#endif
			this_function->total_sources += loop_sample->total_sources;
			srctrg = 0;
			branch_accumulate(&this_function->sources, this_branch, loop_sample, this_function, srctrg);
#ifdef DBUG
			fprintf(stderr,"function_accumulate: this_function->sources = %p\n",this_function->sources);
			fprintf(stderr,"function_accumulate: &this_function->sources = %p\n",&this_function->sources);
#endif
			}

//			aggregate call_list into functions sources list
		this_branch = loop_sample->call_list;
		if(this_branch != NULL)
			{
#ifdef DBUG
			fprintf(stderr,"call_list non zero for function %s, at address 0x%"PRIx64"\n",
				this_function->function_name,loop_sample->rva);
			fprintf(stderr," call_list count = %d from address 0x%"PRIx64"\n",this_branch->count, this_branch->address);
			fprintf(stderr,"function_accumulate: &this_function->targets = %p\n",&this_function->targets);
#endif
			this_function->total_targets += loop_sample->total_targets;
			srctrg = 1;
			branch_accumulate(&this_function->targets, this_branch, loop_sample, this_function, srctrg);
#ifdef DBUG
			fprintf(stderr,"function_accumulate: this_function->targets = %p\n",this_function->targets);
			fprintf(stderr,"function_accumulate: &this_function->targets = %p\n",&this_function->targets);
#endif
			}

		if(this_function != NULL)this_function->total_sample_count += loop_sample->total_sample_count;
#ifdef DBUG
		if(strcmp(this_function->function_name, "context_switch.isra.59") == 0)
				printf_rva(loop_sample, this_function, this_function->this_process);
#endif
		rva_count++;
		total_rva_count++;
		total_function_sample_count += loop_sample->total_sample_count;
		global_sample_count_in_func += loop_sample->total_sample_count;
		if(rva_count == 1)old_function = this_function;
		loop_sample = loop_sample->next;
		}
	return;
}

pointer_data * 
sort_global_func_list(void)
{
	pointer_data * global_func_list;
	function_struc_ptr loop_function, this_function;
	int i,j,k, func_limit;
	double summed_samples,total_samples;
	module_struc_ptr this_module;
	process_struc_ptr this_process;
	char *old_name, *new_name;
	char demangle[] = "c++filt ";
	size_t demangle_len = 8, funcname_len, line_buf_len = 1024;
	char * funcname_cmd, line_buf[1024];

#ifdef DBUG
	fprintf(stderr," in sort_global_func_list, global_func_count = %d\n",global_func_count);
#endif

	global_func_list = (pointer_data *) malloc(global_func_count*sizeof(pointer_data));

	loop_function = global_func_stack;
	i = 0;
	while(loop_function != NULL)
		{
		if(i > global_func_count - 1)
			{
			fprintf(stderr," too many functions in linked list i = %d, global_func_count = %d\n",i,global_func_count);
			err(1,"too many functions in linked list in sort_global_func");
			}
		global_func_list[i].ptr = (sample_struc_ptr) loop_function;
		global_func_list[i].val = loop_function->total_sample_count;
		i++;
		loop_function = loop_function->next;
		}
	if(i != global_func_count)
		{
		fprintf(stderr," too few functions in linked list i = %d, global_func_count = %d\n",i,global_func_count);
		err(1,"too few functions in linked list in sort_global_func");
		}
	quickSort2(global_func_list,global_func_count);
//	print data on up to the hotteest 10 functions to the log
	func_limit = global_func_count - 10;
	if(func_limit < 1) func_limit=1;
	for(i=global_func_count; i >= func_limit; i--)
		{
		this_function = (function_struc_ptr) global_func_list[i-1].ptr;
		fprintf(stderr," function = %s, total_sample_count = %d\n",this_function->function_name,this_function->total_sample_count);
		}
//	set each function_struc's funclist_index value
	for(i=global_func_count; i>= 1; i--)
		{
		this_function = (function_struc_ptr) global_func_list[i-1].ptr;
		this_function->funclist_index = global_func_count - i;
		}
	total_samples = global_sample_count_in_func + global_branch_sample_count;
#ifdef DBUG
	fprintf(stderr,"sort_global_func: total_samples = %g, global_samp_in_func = %d, global_branch_samp_count = %d\n",
		total_samples, global_sample_count_in_func, global_branch_sample_count);
#endif

	summed_samples = 0.;
	i = global_func_count - 1;

	while((i >= 0) && (summed_samples/total_samples < sum_cutoff))
		{
		this_function = (function_struc_ptr) global_func_list[i].ptr;
		this_module = this_function->this_module;
		this_process = this_function->this_process;
//	demangle function names
//		use BFD  libliberty instead
		old_name = this_function->function_name;
#ifdef DBUG
		fprintf(stderr," old name before demangling = %s\n",old_name);
#endif
		new_name = bfd_demangle(NULL, old_name, DMGL_PARAMS | DMGL_ANSI);
#ifdef DBUG
		fprintf(stderr," new name address after demangling = %p\n",new_name);
		if(new_name != NULL)fprintf(stderr," new name after demangling = %s\n",new_name);
#endif
		if(new_name !=NULL)
			{
			funcname_len = strlen(new_name);
			this_function->function_name = (char*) malloc((funcname_len + 1)*sizeof(char));
			if(this_function->function_name == NULL)
				{
				fprintf(stderr," failed to malloc buffer for demangled func name, module path = %s\n",this_module->path);
				err(1, "failed to malloc buffer for module demangle func name");
				}
			for(j=0; j<funcname_len; j++)this_function->function_name[j] = new_name[j];
			this_function->function_name[funcname_len] = '\0';
//		save mangled name
//			free(old_name);
			}
		free(new_name);
		i--;
		summed_samples+=this_function->total_sample_count;
		}
	return global_func_list;
}

void 
src_trg_func_list(pointer_data * global_func_list)
{
	int i,j,k, retval;
	func_branch_struc_ptr this_branch;
	function_struc_ptr this_function;
	uint64_t this_count;
	pointer_data *branch_array;

	for(i=global_func_count; i>= 1; i--)
		{
		this_function = (function_struc_ptr) global_func_list[i-1].ptr;
//	sort source stack into descending order first
		if(this_function->sources != NULL)
			{
			if(this_function->func_sources == 0)
				err(1," function %s has non null source stack but func_sources = 0",this_function->function_name);
			branch_array = (pointer_data*)malloc(this_function->func_sources*sizeof(pointer_data));
			if(branch_array == NULL)
				err(1,"failed to malloc branch array for function %s i = %d\n",this_function->function_name,i);
			j = 0;
			this_branch = this_function->sources;
			while((this_branch != NULL) && ( j < this_function->func_sources))
				{
				branch_array[j].ptr = (sample_struc_ptr)this_branch;
				branch_array[j].val = this_branch->this_branch_target->count;
				j++;
				this_branch = this_branch->next;
				}
			quickSort2(branch_array,this_function->func_sources);
//			going through the array in order will put the linked list into descending order
			for(j=0; j < this_function->func_sources; j++)
				{
				this_branch = (func_branch_struc_ptr)branch_array[j].ptr;
				if(j == 0) this_branch->next = NULL;
				if(j > 0)this_branch->next = this_function->sources;
				this_function->sources = this_branch;
				}
			free(branch_array);
			}
//	sort target stack into descending order first
		if(this_function->targets != NULL)
			{
			if(this_function->func_targets == 0)
				err(1," function %s has non null target stack but func_targets = 0",this_function->function_name);
			branch_array = (pointer_data*)malloc(this_function->func_targets*sizeof(pointer_data));
			if(branch_array == NULL)
				err(1,"failed to malloc branch array for function %s i = %d\n",this_function->function_name,i);
			j = 0;
			this_branch = this_function->targets;
			while((this_branch != NULL) && ( j < this_function->func_targets))
				{
				branch_array[j].ptr = (sample_struc_ptr)this_branch;
				branch_array[j].val = this_branch->this_branch_target->count;
				j++;
				this_branch = this_branch->next;
				}
			quickSort2(branch_array,this_function->func_targets);
//			going through the array in order will put the linked list into descending order
			for(j=0; j < this_function->func_targets; j++)
				{
				this_branch = (func_branch_struc_ptr)branch_array[j].ptr;
				if(j == 0) this_branch->next = NULL;
				if(j > 0)this_branch->next = this_function->targets;
				this_function->targets = this_branch;
				}
			free(branch_array);
			}
#ifdef DBUG
		fprintf(stderr,"src_trg_func: function %s, has source ptr = %p and target ptr = %p\n",
			this_function->function_name,this_function->sources, this_function->targets);
		if(this_function->sources != NULL)
			fprintf(stderr,"src_trg_func: hottest source = 0x%"PRIx64", with %d counts\n",
				this_function->sources->this_branch_target->address,this_function->sources->this_branch_target->count);
		if(this_function->targets != NULL)
			fprintf(stderr,"src_trg_func: hottest target = 0x%"PRIx64", with %d counts\n",
				this_function->targets->this_branch_target->address,this_function->targets->this_branch_target->count);
#endif
		}				
	return;
}

void 
hotspot_function(pointer_data * global_func_list)
{
	int i,j,k;
	FILE * sh, *filt, *platform;
	char spreadsheet[] = "./spreadsheets/function_hotspots.csv";
	char platform_str[] = "./spreadsheets/platform_properties.txt";
	char function_name[] = "Function Name", offset[] = "Offset", length[] = "Length", module[] = "Module", process[] = "Process";
	process_struc_ptr this_process;
	module_struc_ptr this_module;
	function_struc_ptr this_function, branch_function;
	double summed_samples, total_samples;
	int num_col, num_branches;
	func_branch_struc_ptr this_branch;
	char mode[] = "w+";

#ifdef DBUG
	fprintf(stderr," in hotspot_function\n");
	fprintf(stderr,"hotspot_function num_col = %d, fixed = %d, ordered = %d\n",
		num_col, global_event_order->num_fixed, global_event_order->num_ordered);
#endif
	num_col = global_event_order->num_fixed + global_event_order->num_ordered;

#ifdef DBUG
	fprintf(stderr,"num_col = %d\n",num_col);
#endif
	sh = fopen(spreadsheet, mode);
	if(sh == NULL)
		{
		fprintf(stderr,"failed to open function_hotspot spreadsheet\n");
		err(1,"failed to open hotspot spreadsheet");
		}

//	platform data file
	platform = fopen(platform_str, mode);
	if(platform == NULL)
		{
		fprintf(stderr,"failed to open platform data file\n");
		err(1,"failed to open platform data");
		}
	fprintf(platform," Format_version: 1.0\n");
	fprintf(platform," architecture:%s\n",arch);
	fprintf(platform," family:%d, model:%d\n",family,model);
	fprintf(platform," num_sockets:%d, num_cores:%d\n",num_sockets,num_cores);
	fprintf(platform," num_col:%d\n",num_col);
	fprintf(platform," cpu_desc:%s\n",cpu_desc);
	fprintf(platform," num_fixed:%d\n",global_event_order->num_fixed);
	fprintf(platform," num_ordered:%d\n",global_event_order->num_ordered);
	fprintf(platform," num_branch:%d\n",global_event_order->num_branch);
	fprintf(platform," num_sub_branch:%d\n",global_event_order->num_sub_branch);


//header rows of the hotspot table

	fprintf(sh,"[\n");
//	fprintf(sh,"[null,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",",function_name,offset,length,module,process);
	fprintf(sh,"[, , , \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",function_name,offset,length,module,process);
//	for(i=0; i < num_events; i++)fprintf(sh," \"%s\",",event_list[i].name);
	for(i=0; i < num_col; i++)fprintf(sh," \"%s\",",global_event_order->order[i].name);
//	fprintf(sh,"null],\n");
	fprintf(sh," ],\n");
	fprintf(sh,"[, , , \"0:4\",");
	for(k=1;k<5;k++)fprintf(sh," \"0_%d:0\",",k);
	for(k=0;k<num_col;k++)fprintf(sh,"\"%d%s\",",1+global_event_order->order[k].base_col,global_event_order->order[k].ctrl_string);
	fprintf(sh," ],\n");
//	fprintf(sh,"[null,\"MSR Programmings\",null,null,null,null,");
	fprintf(sh,"[, , , \"MSR Programmings\", null, null, null, null,");
//	for(i=0; i < num_events; i++)fprintf(sh," \"0x%"PRIx64"\",",global_attrs[i].attr.config);
//	for(i=0; i < num_col; i++)fprintf(sh," \"0x%"PRIx64"\",",global_event_order->order[i].config);
	for(i=0; i < num_col; i++)fprintf(sh,"0x%"PRIx64",",global_event_order->order[i].config);
//	fprintf(sh,"null],\n");
	fprintf(sh," ],\n");
//	fprintf(sh,"[null,\"Period\",null,null,null,null,");
	fprintf(sh,"[, , , \"Period\", , , , ,");
//	for(i=0; i < num_events; i++)fprintf(sh," %d,",global_attrs[i].attr.sample.sample_period);
	for(i=0; i < num_col; i++)fprintf(sh," %ld,",global_event_order->order[i].Period);
//	fprintf(sh,"null],\n");
	fprintf(sh," ],\n");
//	fprintf(sh,"[null,\"Multiplex\",null,null,null,null,");
	fprintf(sh,"[, , , \"Multiplex\", , , , ,");
//	for(i=0; i < num_events; i++)fprintf(sh," %5.4lf,",global_multiplex_correction[num_events*(num_cores+num_sockets) + i]);
	for(i=0; i < num_col; i++)fprintf(sh," %5.4lf,",global_event_order->order[i].multiplex);
//	fprintf(sh,"null],\n");
	fprintf(sh," ],\n");
	fprintf(sh,"[, , , \"Penalty\", , , , ,");
	for(k=0; k < num_col; k++)fprintf(sh," %d,",global_event_order->order[k].penalty);
	fprintf(sh," ],\n");
	fprintf(sh,"[, , , \"Cycles\", , , , ,");
	for(k=0; k < num_col; k++)fprintf(sh," %d,",global_event_order->order[k].cycle);
	fprintf(sh," ],\n");

	total_samples = global_sample_count_in_func + global_branch_sample_count;
#ifdef DBUG
	fprintf(stderr," finished spreadsheet header rows\n");
	fprintf(stderr,"hotspot_function: total_samples = %g, global_samp_in_func = %d, global_branch_samp_count = %d\n",
		total_samples, global_sample_count_in_func, global_branch_sample_count);
#endif

	summed_samples = 0.;
	i = global_func_count - 1;
	fprintf(stderr,"hotspot_function: global_func_count = %d, func_cutoff = %d\n",global_func_count, func_cutoff);
	while((i >= 0) && (summed_samples/total_samples < sum_cutoff))
		{
		this_function = (function_struc_ptr) global_func_list[i].ptr;
		this_module = this_function->this_module;
		this_process = this_function->this_process;
#ifdef DBUG
		fprintf(stderr,"i = %d, name = %s, addr = 0x%"PRIx64", len = %ld, num_sources = %d, num_targets = %d\n",
			i,this_function->function_name,this_function->function_rva_start,this_function->function_length,
			this_function->func_sources,this_function->func_targets);
#endif

		fprintf(sh,"[,%d,%d,\"%s\",\"0x%"PRIx64"\",\"0x%"PRIx64"\",",
			this_function->funclist_index,this_function->funclist_index,
			this_function->function_name,this_function->function_rva_start,this_function->function_length);
		fprintf(sh," \"%s\", \"%s\",",this_module->module_name,this_process->name);
#ifdef DBUG
		if(i == global_func_count - 1)
			{
			fprintf(stderr,"hotspot_function global_event_order data before branch_eval\n");
			for(k=0; k<num_events;k++)
				{
				fprintf(stderr,"name = %s, total = %d,",event_list[k].name,this_function->sample_count[num_events*(num_cores+num_sockets) + i ] );
				for(j=0; j<num_cores; j++)
					fprintf(stderr," %d,",this_function->sample_count[num_cores*k + j]);
				fprintf(stderr,"\n");
				}
			for(j=0; j<num_col; j++)fprintf(stderr," name = %s, index = %d, %d\n",global_event_order->order[j].name,global_event_order->order[j].index,this_function->sample_count[ global_event_order->order[j].index ]);
			}
#endif
//		this may have been invoked in func_asm
		if(this_function->called_branch_eval == 0)branch_eval(this_function->sample_count);
		for(j=0; j<num_col; j++)fprintf(sh," %d,",this_function->sample_count[ global_event_order->order[j].index ]);
		fprintf(sh," ],\n");
		if(i > global_func_count - func_cutoff)
			{
//	print out the most active sources and targets
//		sources first
#ifdef DBUG
			fprintf(stderr,"hotspot_function: starting source printout\n");
#endif
			this_branch = this_function->sources;
			num_branches = this_function->func_sources;
			j = 0;
			while((this_branch != NULL) && (j < max_branch))
				{
				if(source_column == 0)
					err(1,"source_column not set in hotspot_function loop over sources\n");
				branch_function = this_branch->this_branch_target->this_function;
#ifdef DBUG
				fprintf(stderr,"function %s, this_branch_target address = %p, num_branches = %d\n",
					this_function->function_name,this_branch->this_branch_target, num_branches);
				fprintf(stderr," address = 0x%"PRIx64", count = %d\n",
					this_branch->this_branch_target->address,this_branch->this_branch_target->count);
				fprintf(stderr,"branch function address = %p\n",branch_function);
				if(branch_function != NULL)
					fprintf(stderr," index = %d, index = %d, name = %s\n",
						this_function->funclist_index,branch_function->funclist_index,branch_function->function_name);
#endif
				if(branch_function == NULL)
					{
					fprintf(sh,"[,%d,-1,,\"0x%"PRIx64"\",,\"%s\",,",
						this_function->funclist_index,this_branch->this_branch_target->address,
							this_branch->this_branch_target->this_module->module_name);
					}
				else
					{
					fprintf(sh,"[,%d,%d,\"%s\",\"0x%"PRIx64"\",,\"%s\",,",
						this_function->funclist_index,branch_function->funclist_index,branch_function->function_name,
						this_branch->this_branch_target->address,this_branch->this_branch_target->this_module->module_name);
					}
				for(k=0; k<source_column; k++)fprintf(sh,",");
				fprintf(sh,"%d,",this_branch->this_branch_target->count);
				for(k=source_column+1; k< num_col; k++)fprintf(sh,",");
				fprintf(sh," ],\n");
				j++;
				this_branch = this_branch->next;
				}
//		then targets
#ifdef DBUG
			fprintf(stderr,"hotspot_function: starting target printout\n");
#endif
			this_branch = this_function->targets;
			num_branches = this_function->func_targets;
			j = 0;
			while((this_branch != NULL) && (j < max_branch))
				{
				if(target_column == 0)
					err(1,"target_column not set in hotspot_function loop over sources\n");
				branch_function = this_branch->this_branch_target->this_function;
#ifdef DBUG
				fprintf(stderr,"function %s, this_branch_target address = %p, num_branches = %d\n",
					this_function->function_name,this_branch->this_branch_target, num_branches);
				fprintf(stderr," address = 0x%"PRIx64", count = %d\n",
					this_branch->this_branch_target->address,this_branch->this_branch_target->count);
				fprintf(stderr,"branch function address = %p\n",branch_function);
				if(branch_function != NULL)
					fprintf(stderr," index = %d, index = %d, name = %s\n",
						this_function->funclist_index,branch_function->funclist_index,branch_function->function_name);
#endif
				if(branch_function == NULL)
					{
#ifdef DBUG
					fprintf(stderr," branch_function = NULL\n");
					fprintf(stderr,"[,%d,-1,\"0x%"PRIx64"\",,,,,",
						this_function->funclist_index,this_branch->this_branch_target->address);
#endif
					fprintf(sh,"[,%d,-1,,\"0x%"PRIx64"\",,\"%s\",,",
						this_function->funclist_index,this_branch->this_branch_target->address,
							this_branch->this_branch_target->this_module->module_name);
					}
				else
					{
#ifdef DBUG
					fprintf(stderr," branch_function != NULL\n");
					fprintf(stderr,"[,%d,%d,\"%s\",,,,,",
						this_function->funclist_index,branch_function->funclist_index,branch_function->function_name);
#endif
					fprintf(sh,"[,%d,%d,\"%s\",\"0x%"PRIx64"\",,\"%s\",,",
						this_function->funclist_index,branch_function->funclist_index,branch_function->function_name,
						this_branch->this_branch_target->address,this_branch->this_branch_target->this_module->module_name);
					}
				for(k=0; k<target_column; k++)fprintf(sh,",");
				fprintf(sh,"%d,",this_branch->this_branch_target->count);
				for(k=target_column+1; k< num_col; k++)fprintf(sh,",");
				fprintf(sh," ],\n");
				j++;
				this_branch = this_branch->next;
				}
			}
#ifdef DBUG
		if(i == global_func_count - 1)
			{
			fprintf(stderr,"hotspot_function global_event_order data\n");
			for(j=0; j<num_col; j++)fprintf(stderr," name = %s, index = %d, %d\n",global_event_order->order[j].name,global_event_order->order[j].index,this_function->sample_count[ global_event_order->order[j].index ]);
			}
#endif
		i--;
		summed_samples+=this_function->total_sample_count;
#ifdef DBUG
		fprintf(stderr,"hotspot_function: i = %d,summed_samples = %g, total_samples = %g, sum_cutoff = %g\n",
			i, summed_samples, total_samples, sum_cutoff);
#endif
		}
//	final line feed for last row of function data ...
	fprintf(sh,"\n");
	branch_eval(global_sample_count);
	fprintf(sh,"[,\"Global sample breakdown\",,,,,\"all_modules\",\"all_processes\",");
	for(j=0; j<num_col; j++)fprintf(sh," %d,",global_sample_count[ global_event_order->order[j].index ]);
	fprintf(sh," ]\n");
	fprintf(sh,"]\n");
	fclose(sh);
	fclose(platform);
}

void
hotspot_call_graph(pointer_data * global_func_list)
{
	FILE *dot;
	int i,j,k,l,m,n,count,max_count=0,node_count,link_count,hot_func_limit, max_sample_count,max_link_count;
	int fillcolor, penwidth,label,dot_len,svg_len,pos;
	float total_samples,summed_samples;
	uint64_t address,old_node;
	char dot_file[]="spreadsheets/cg/0_cg.dot", svg_file[]="spreadsheets/cg/0_cg.svg";
	char mode[] = "w+";
	char * svg_cmd;
	char short_name[13];
	size_t name_len, max_name_len = 12;
	process_struc_ptr this_process, main_process;
	module_struc_ptr this_module, branch_module;
	function_struc_ptr this_function;
	functionlist_struc_ptr	function_list;
	func_branch_struc_ptr this_source, this_target;
	branch_struc_ptr this_branch;
	typedef struct linklist_struc{
		function_struc_ptr	source;
		function_struc_ptr	target;
		int			count;
		}linklist_data;
	linklist_data *link_data;
	linkpairs_data *linkpairs, *node_list;
	uint64_t  mask = 0xFFFFFFFF;
	uint64_t previous_link;
	int to_links,from_links,ret_val;

	total_samples = global_sample_count_in_func + global_branch_sample_count;
	summed_samples = 0.;
	main_process = principal_process_stack;
	fprintf(stderr,"hotspot_call_graph: main process is %s\n",main_process->name);
	node_count = 0;
	link_count = 0;
	max_sample_count = 0;
	max_link_count = 0;

	i = global_func_count - 1;
	hot_func_limit = global_func_count - func_cutoff;
	if(hot_func_limit < 0) hot_func_limit=0;

#ifdef DBUG
	fprintf(stderr," total_samples = %g, sum_cutoff = %g, func_cutoff = %d, hot_func_limit = %d, initial i = %d\n",
		total_samples, sum_cutoff, func_cutoff, hot_func_limit,i);
#endif

	while((i >= hot_func_limit) && (summed_samples/total_samples < sum_cutoff))
		{
		this_function = (function_struc_ptr) global_func_list[i].ptr;
		count = global_func_list[i].val;
		summed_samples += count;
		this_module = this_function->this_module;
		this_process = this_function->this_process;
#ifdef DBUG
		fprintf(stderr," hotspot_call_graph: function %s has sample count %d from module %s, process %s\n",
			this_function->function_name,count,this_module->module_name,this_process->name);
#endif
		if(this_process != main_process)
			{
			i--;
			continue;
			}
		if(this_function->total_sample_count > max_sample_count)max_sample_count = this_function->total_sample_count;
		node_count++;
		if(count > max_count)max_count = count;
		this_source = this_function->sources;
		from_links = 0;
#ifdef DBUG
		fprintf(stderr," sources for function %s, total source = %d, this_source = %p\n",
			this_function->function_name,this_function->total_sources,this_source);
#endif
		l = 0;
		while(this_source != NULL)
			{
			this_branch = this_source->this_branch_target;
			branch_module = this_branch->this_module;
			function_list = branch_module->function_list;
			address = this_branch->address;
			if(function_list != NULL)
				{
				this_branch->this_function = module_binsearch(address,function_list);
				if(this_branch->this_function != NULL)
					{
					if(this_branch->this_function->total_sample_count > max_sample_count)
						max_sample_count = this_branch->this_function->total_sample_count + 1;
					if(this_branch->count > max_link_count)
						max_link_count = this_branch->count + 1;
#ifdef DBUG
					fprintf(stderr," was called from 0x%"PRIx64", in function %s, %d times\n",
						this_branch->address,this_branch->this_function->function_name,this_branch->count);
#endif
					}
				else
					{
#ifdef DBUG	
					fprintf(stderr," was called from 0x%"PRIx64", in function NULL, %d times\n",
						this_branch->address,this_branch->count);
#endif
					}
				}
			from_links++;
			link_count++;
			l++;
			if((this_function->total_sources > source_cutoff) && (l > 11))break;
			this_source = this_source->next;
			}
		to_links = 0;
		this_source = this_function->targets;
#ifdef DBUG
		fprintf(stderr," targets for function %s\n",this_function->function_name);
#endif
		while(this_source != NULL)
			{
			this_branch = this_source->this_branch_target;
#ifdef DBUG
			fprintf(stderr," was re-entered from 0x%"PRIx64", %d times\n",this_branch->address,this_branch->count);
#endif
			branch_module = this_branch->this_module;
			function_list = branch_module->function_list;
			address = this_branch->address;
#ifdef DBUG
			fprintf(stderr," calling module_binsearch for address 0x%"PRIx64"\n",address);
#endif
			if(function_list != NULL)
				{
				this_branch->this_function = module_binsearch(address,function_list);
				if(this_branch->this_function != NULL)
					{
					if(this_branch->this_function->total_sample_count > max_sample_count)
						max_sample_count = this_branch->this_function->total_sample_count + 1;
					if(this_branch->count > max_link_count)
						max_link_count = this_branch->count + 1;
#ifdef DBUG
					fprintf(stderr," was called from 0x%"PRIx64", in function %s, %d times\n",
						this_branch->address,this_branch->this_function->function_name,this_branch->count);
#endif
					}
				else
					{
#ifdef DBUG
					fprintf(stderr," was called from 0x%"PRIx64", in function NULL, %d times\n",
						this_branch->address,this_branch->count);
#endif
					}
				}
			to_links++;
			link_count++;
			this_source = this_source->next;
			}
		i--;
#ifdef DBUG
		fprintf(stderr," links: from_links = %d, to_links = %d, function %s\n",to_links,from_links,this_function->function_name);
		fprintf(stderr,"summed samples = %g, total_samples = %g\n",summed_samples,total_samples);
#endif
		}

#ifdef DBUG
	fprintf(stderr," end of first loop over functions (and sources/targets, node_count = %d, link_count = %d\n",node_count,link_count);
#endif
	if(max_sample_count == 0)
		err(1,"hotspot_call_graph: bad data file, none of the top functions are in the dominant process, max_sample_count = 0");
//	create arrays for node lists, lists of links and the linkpair structure used for eliminating duplicates
	node_list = (linkpairs_data *) malloc((link_count+node_count)*sizeof(linkpairs_data));
	if(node_list == NULL)
		err(1,"failed to malloc node_list in hotspot_call_graph");
	link_data = (linklist_data *) malloc((link_count)*sizeof(linklist_data));
	if(link_data == NULL)
		err(1,"failed to malloc link_data in hotspot_call_graph");

//	linkpairs is used to eliminated duplicate link entries due to building the graph from both
//	the sources and targets linked lists for each function.
//	sources and targets are both used to pick up nodes on the edges of the graph and leaf nodes
//	the duplicates are identified by creating a uint64_t with the lower 32 bits of the 
//	source and target function_struc_ptr's and packing them into the upper and lower 32 bits of a uint64_t. 
//	The order is reversed between source links and target links to ensure
//	that duplicate links have identical values for the packed bit string.
//	The linkpair structure has the original index of the link which is used to access the link_data array.
//	When the array of structures is sorted, duplicate links will be adjacent
//	thus the fprintf's to the dot file only occur with the previous linkpair uint64_t (src_trg) is
//	different from the current value
//	this reduces a quadratic pairwise analysis to a quicksort
//	The structure of a uint64_t and int is also useful for holding onto the original position in the function table
//	of the node allowing the display to link the function table to the call graph
	linkpairs = (linkpairs_data *) calloc(1,(link_count)*sizeof(linkpairs_data));
	if(linkpairs == NULL)
		err(1,"failed to malloc linkpairs in hotspot_call_graph");

//	second pass though all the function branch structures
//	construct node and link arrays
	j = 0;
	k = 0;
	i = global_func_count - 1;
	summed_samples = 0.;

	while((i >= hot_func_limit) && (summed_samples/total_samples < sum_cutoff))
		{
		this_function = (function_struc_ptr) global_func_list[i].ptr;
		count = global_func_list[i].val;
		summed_samples += count;
		this_module = this_function->this_module;
		this_process = this_function->this_process;
#ifdef DBUG
		fprintf(stderr," hotspot_call_graph: function %s has sample count %d from module %s, process %s\n",
			this_function->function_name,count,this_module->module_name,this_process->name);
#endif
		if(this_process != main_process)
			{
			i--;
			continue;
			}
		node_list[j].src_trg = (uint64_t)this_function;
		node_list[j].index = global_func_count - i - 1;
		j++;
		this_source = this_function->sources;
#ifdef DBUG
		fprintf(stderr," sources for function %s\n",this_function->function_name);
#endif
		l = 0;
		while(this_source != NULL)
			{
			this_branch = this_source->this_branch_target;
			node_list[j].src_trg = (uint64_t)this_branch->this_function;
			node_list[j].index = -1;
			j++;
			if(j > node_count+link_count)
				err(1,"too many nodes in second loop in hotpsot_call_graph, j = %d, node_count + link_count = %d",
					j,node_count+link_count);
			link_data[k].source = this_function;
			link_data[k].target = this_branch->this_function;
			link_data[k].count = this_branch->count;
			linkpairs[k].src_trg = (uint64_t) ((uint64_t)link_data[k].source & mask);
			linkpairs[k].src_trg += (uint64_t)(((uint64_t)link_data[k].target & mask)<<32);
			linkpairs[k].index = k;
			k++;
			if(k > link_count)
				err(1,"too many links in second loop in hotpsot_call_graph, k = %d, link_count = %d",j,link_count);
			l++;
			if((this_function->total_sources > source_cutoff) && (l > 11))break;
			this_source = this_source->next;
			}
		this_source = this_function->targets;
#ifdef DBUG
		fprintf(stderr," targets for function %s\n",this_function->function_name);
#endif
		while(this_source != NULL)
			{
			this_branch = this_source->this_branch_target;
			node_list[j].src_trg = (uint64_t)this_branch->this_function;
			node_list[j].index = -1;
			j++;
			if(j > node_count+link_count)
				err(1,"too many nodes in second target loop in hotpsot_call_graph, j = %d, node_count + link_count = %d",
					j,node_count+link_count);
			link_data[k].target = this_function;
			link_data[k].source = this_branch->this_function;
			link_data[k].count = this_branch->count;
			linkpairs[k].src_trg = (uint64_t) ((uint64_t)link_data[k].source & mask);
			linkpairs[k].src_trg += (uint64_t)(((uint64_t)link_data[k].target & mask)<<32);
			linkpairs[k].index = k;
			k++;
			if(k > link_count)
				err(1,"too many links in second target loop in hotpsot_call_graph, k = %d, link_count = %d",j,link_count);
			this_source = this_source->next;
			}
		i--;
		}

#ifdef DBUG
	fprintf(stderr," end of second loop over functions (and sources/targets, node_count/j = %d, link_count/k = %d\n",j,k);
#endif

//	sort the node list
	if(j != node_count+link_count)
		err(1,"second loop did not find as many nodes as the first j = %d, node_count + link_count = %d\n",j,node_count+link_count);
	quickSort_link(node_list,node_count+link_count);
	quickSort_link(linkpairs,link_count);

#ifdef DBUG
	fprintf(stderr,"back from sorters\n");
#endif

//	open and print out the dot file
	dot = fopen(dot_file,mode);
	fprintf(dot,"digraph \"%s\"{\n",main_process->name);
	fprintf(dot,"\trankdir=LR;\n");
	fprintf(dot,"\tnode[shape=oval,colorscheme=ylorrd9, style=filled];\n\n");

//	the function name shown in the CG must be cut to a short number of characters
//	We use 12 as the limit taking 5 from the front and 5 from the back with 2 spaces
//	To ensure that the position in the function table is found,
//	all the nodes with the same function_struc_ptr value are checked for a non "-1" value
//	There can only be one
	old_node = node_list[0].src_trg;
	if(old_node != 0)
		{
		pos = -1;
		if(node_list[0].index != -1)pos = node_list[0].index;
		l = 1;
		while((old_node == node_list[l].src_trg) && (l < j))
			{
			if(node_list[l].index != -1)pos = node_list[l].index;
			l++;
			}
		this_function = (function_struc_ptr)old_node;
		name_len = strlen(this_function->function_name);
		if(name_len >= max_name_len)
			{
			for(m=0;m<max_name_len;m++)short_name[m] = ' ';
			for(m=0;m<5;m++)short_name[m] = this_function->function_name[m];
			for(m=0;m<6;m++)short_name[max_name_len - 1 - m] = this_function->function_name[name_len - m];
			short_name[max_name_len] = '\0';
			}
		else
			{
#ifdef DBUG
			fprintf(stderr,"name_len < cuttoff, %lu, name = %s\n",name_len,this_function->function_name);
#endif
			for(m=0;m<name_len;m++)short_name[m] = this_function->function_name[m];
			short_name[name_len] = '\0';
			}
//		fprintf(stderr," short_name = %s\n",short_name);
		fillcolor = 10*(this_function->total_sample_count)/max_sample_count + 1;
		if(fillcolor >= 10) fillcolor=9;
		if(fillcolor == 0) fillcolor=1;
		fprintf(dot,"\t\"%s\" [fillcolor=%d, URL=\"%d\", label=\"%s\"];\n",this_function->function_name,fillcolor,pos,short_name);
#ifdef DBUG
		if(pos != -1)
		fprintf(stderr,"\t\"%s\" [fillcolor=%d, URL=\"%d\", label=\"%s\"];\n",this_function->function_name,fillcolor,pos,short_name);
		fprintf(stderr," func %s at 0x%"PRIx64", struc at %p, pos = %d, name_len = %lu, short_name = %s\n",
			this_function->function_name,this_function->function_rva_start,this_function,pos,name_len,short_name);
#endif
		}
	for(i=1;i<j;i++)
		{
		if(node_list[i].src_trg != old_node)
			{
			old_node = node_list[i].src_trg;
			if(old_node != 0)
				{
				pos = -1;
				if(node_list[i].index != -1)pos = node_list[i].index;
				l = i+1;
				while((old_node == node_list[l].src_trg) && (l < j))
					{
					if(node_list[l].index != -1)pos = node_list[l].index;
					l++;
					}
				this_function = (function_struc_ptr)old_node;
				name_len = strlen(this_function->function_name);
				if(name_len >= max_name_len)
					{
					for(m=0;m<max_name_len;m++)short_name[m] = ' ';
					for(m=0;m<5;m++)short_name[m] = this_function->function_name[m];
					for(m=0;m<6;m++)short_name[max_name_len - 1 - m] = this_function->function_name[name_len - m];
					short_name[max_name_len] = '\0';
					}
				else
					{
#ifdef DBUG
					fprintf(stderr,"name_len < cuttoff, %lu, name = %s\n",name_len,this_function->function_name);
#endif
					for(m=0;m<name_len;m++)short_name[m] = this_function->function_name[m];
					short_name[name_len] = '\0';
					}
				this_function = (function_struc_ptr)old_node;
				fillcolor = 10*(this_function->total_sample_count)/max_sample_count + 1;
				if(fillcolor >= 10) fillcolor=9;
				if(fillcolor == 0) fillcolor=1;
				fprintf(dot,"\t\"%s\" [fillcolor=%d, URL=\"%d\", label=\"%s\"];\n",
					this_function->function_name,fillcolor,pos,short_name);
#ifdef DBUG
				if(pos != -1)
					fprintf(stderr,"\t\"%s\" [fillcolor=%d, URL=\"%d\", label=\"%s\"];\n",
						this_function->function_name,fillcolor,pos,short_name);
				fprintf(stderr," func %s at 0x%"PRIx64", struc at %p, pos = %d, name_len = %lu, short_name = %s\n",
					this_function->function_name,this_function->function_rva_start,this_function,pos,name_len,short_name);
#endif
				}
			}
		}
//`	return;
	fprintf(dot,"\n");
	previous_link = linkpairs[0].src_trg;
	j = linkpairs[0].index;
	if((link_data[j].source != NULL) && (link_data[j].target != NULL))
		{
		penwidth = 9*link_data[j].count/max_link_count + 1;
		if(penwidth > 10)penwidth = 10;
		fprintf(dot,"\t\"%s\"->\"%s\" [penwidth = %d, label=\"%d\"];\n",
			link_data[j].target->function_name,link_data[j].source->function_name,penwidth,link_data[j].count);
		}
	for(i=0;i<k;i++)
		{
		if(linkpairs[i].src_trg == previous_link)continue;
		previous_link = linkpairs[i].src_trg;
		j = linkpairs[i].index;
		if(link_data[j].source == NULL)continue;
		if(link_data[j].target == NULL)continue;
		penwidth = 9*link_data[j].count/max_link_count + 1;
		if(penwidth > 10)penwidth = 10;
		fprintf(dot,"\t\"%s\"->\"%s\" [penwidth = %d, label=\"%d\"];\n",
			link_data[j].target->function_name,link_data[j].source->function_name,penwidth,link_data[j].count);
		}
	fprintf(dot,"}\n");
	fclose(dot);
	dot_len = strlen(dot_file);
	svg_len = strlen(svg_file);
	svg_cmd = (char*)malloc(20+dot_len+svg_len);
	sprintf(svg_cmd,"dot -Tsvg %s > %s\0",dot_file,svg_file);
	ret_val = system(svg_cmd);
	if(ret_val == -1)fprintf(stderr,"system call of dot -Tsvg failed in hotspot_call_graph");
	free(link_data);
	free(linkpairs);
	free(node_list);
	return;
}
void
ppc64_branch_function(char* field2)
{
	char *endptr;
	int instruction_len;
	uint32_t instruction_val;
	uint32_t base, AA, LK, XL;
	int ibm_inst_len = 32;
//	uint32_t aa_mask = 0x80000000;
//	uint32_t lk_mask = 0x40000000;
//	uint32_t base_mask = 0x3F;
//	uint32_t xl_form_mask = 0x7FE00000;
	uint32_t aa_mask = 0x2;
	uint32_t lk_mask = 0x1;
	uint32_t base_mask = 0xFC000000;
	uint32_t xl_form_mask = 0x7FE;
	uint32_t ret_val = 16, ind_val = 2528;
	uint32_t ret_encode = 0x4E800000, ret_mask = 0xFFFF0000;

	branch_type.branch = 0;
	branch_type.call = 0;
	branch_type.ret = 0;
	branch_type.conditional = 0;
	branch_type.indirect_call = 0;
	branch_type.indirect_jmp = 0;

	instruction_len = strlen(field2);
//	fprintf(stderr," encoded instruction length = %d\n",instruction_len);
//	if(instruction_len != ibm_inst_len)return;

	instruction_val = (uint32_t)strtoll(field2, &endptr, 16);
	base = ((base_mask & instruction_val) >> 26);
	AA = aa_mask & instruction_val;
	LK = lk_mask & instruction_val;
	XL = ((xl_form_mask & instruction_val) >> 1);
//	fprintf(stderr," inst_val = 0x%x, base = 0x%x, AA = 0x%x, AK = 0x%x, XL = 0x%x\n",
//		instruction_val,base,AA,LK,XL);	

	if((base == 16) && (LK == 0))
		{
		branch_type.branch = 2;
		branch_type.conditional = 1;
		return;
		}

	if((base == 18) && (LK == 0))
		{
		branch_type.branch = 2;
		return;
		}

	if((base == 16) && (LK != 0))
		{
		branch_type.branch = 2;
		branch_type.conditional = 1;
		branch_type.call = 1;
		return;
		}

	if((base == 18) && (LK != 0))
		{
		branch_type.branch = 2;
		branch_type.call = 1;
		return;
		}

	if((instruction_val & ret_mask) == ret_encode)
		{
		branch_type.branch = 2;
		branch_type.ret = 1;
		return;
		}
	if((base == 19) && (XL == 16))
		{
		branch_type.branch = 2;
		branch_type.call = 1;
		branch_type.indirect_call = 1;
		return;
		}

	if((base == 19) && (XL == 528))
		{
		branch_type.branch = 2;
		branch_type.indirect_jmp = 1;
		return;
		}
}
void
arm32_branch_function(char* field2)
{
	int instruction_len;
	branch_type.branch = 0;
	branch_type.call = 0;
	branch_type.conditional = 0;
	branch_type.indirect_call = 0;
	branch_type.indirect_jmp = 0;

	instruction_len = strlen(field2);
//	fprintf(stderr," encoded instruction length = %d\n",instruction_len);

	if(instruction_len == 4)
		{
	// ARM ARM A8.8.18
	// B branches
	// B (T1 encoding -	16-bit [1101|xxxx|xxxx|xxxx] == [d|x|x|x])
	// B (T2 encoding -	16-bit [1110|0xxx|xxxx|xxxx] == [e|<8|x|x])
		if (field2[0] == 'd')
			{
				  branch_type.branch = 2; /* why 2? */
				  branch_type.conditional = 1; 
			}

		if ( (field2[0] == 'e' ) && (field2[1] - '0') < 8)
			{
			  // B branches
				  branch_type.branch = 2; /* why 2? */
			}
//	fprintf(stderr,"test 1 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.29
	// CBZ/CBNZ compare and branch
	// (T1 encoding - 16-bit [1011|x0x1|xxxx|xxxx])
		if ( ( (field2[0] == 'b' ) && ( ( ((field2[1] - '0') >= 1) && ( (field2[1] - '0') <= 3)) || ( (field2[1] - '0') == 9 ) ) ) ||
		     ( (field2[0] == 'b' ) &&  ( (field2[1] - 'a') == 1) )
			) 
			{
		 // CBZ/CBNZ branches
			 branch_type.branch = 2;
			}
//	fprintf(stderr,"test 2 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.27 & A8.8.28    
	// BX/BXJ branch and exchange
	// BX  (T1 encoding - 16-bit [0100|0111|0xxx|xxxx])
	// BXJ (T1 encoding - 16-bit [1111|0011|1100|xxxx])
		if (( (field2[0] == '4') && (field2[1] == '7') && ( (field2[3] - '0') < 8 ) ) ||
			 ( (field2[0] == 'f') && (field2[1] == '3') && (field2[3] == 'c'))
			) 
			{
		 // BX/BXJ branches
			 branch_type.branch = 2;
			}
//	fprintf(stderr,"test 3 branch_type.branch = %d\n",branch_type.branch);

	// BLX reg (T1 encoding 16-bit [0100|0111|1xxx|xxxx])
		if ((field2[0] == '4' && (field2[1] < '8' && field2[1] >= '0') && ( (field2[2] - '0' == 8) && (field2[2] - '0' == 9)  ) ) ||
		    (field2[0] == '4' && (field2[1] < '8' && field2[1] >= '0') && ( (field2[2] - 'a' >= 0) && (field2[2] - 'a' <= 5)  ) ) 
			)
			{
			 branch_type.call = 1;
			 branch_type.branch = 2;
			}
//	fprintf(stderr,"test 6 branch_type.branch = %d\n",branch_type.branch);
		}
	if(instruction_len == 8)
		{
		if(
	// B (T3/T4 encoding - 16+16-bit [1111|0xxx|xxxx|xxxx][10xx|xxxx|xxxx|xxxx])
	// B (A1 encoding -	32-bit [xxxx|1010|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
			 ( (field2[0] == 'f') && ( (field2[1] -'0') < 8) && ( (field2[4] - '0') == 8 || (field2[4] - '0') == 9)) ||
			 ( (field2[0] == 'f') && ( (field2[1] -'0') < 8) && ( (field2[4] - 'a') == 0 || (field2[4] - 'a') == 1)) ||
			 (field2[1] == 'a') ||
	// BX  (A1 encoding - 32-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0001|xxxx])
	// BXJ (A1 encoding - 16-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0010|xxxx])
			 ( (field2[1] == '1') && (field2[3] == '2' ) && ( (field2[6] == '1') || (field2[6] == '2') ) ) || 
	// BLX reg (A1 encoding 32-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0011|xxxx])
			 (field2[1] == '1' && field2[2] == '2' && field2[6] == '3') || 

	// ARM ARM A8.8.236
	// TBB/TBH table branch
	// TBB/TBH (T1 encoding 16+16-bit [1110|1000|1101|xxxx][xxxx|xxxx|000x|xxxx])
			( (field2[0] == 'e') && (field2[1] == '8') && (field2[3] == 'd') && ( (field2[6] == '0') ||  (field2[6] == '1') ) )
			)
			{
			 branch_type.branch = 2;
			}
//	fprintf(stderr,"test 4 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.26
	// ARM ARM A8.8.25
	// BL/BLX calls
	// BL/BLX (T1 encoding 16+16-bit [1111|0xxx|xxxx|xxxx][11x1|xxxx|xxxx|xxxx])
	// BL/BLX (T2 encoding 16+16-bit [1111|0xxx|xxxx|xxxx][11x0|xxxx|xxxx|xxxx])
	// BL  (A1 encoding 32-bit [xxxx|1011|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
	// BLX (A1 encoding 32-bit [1111|101x|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
		if ( ( (field2[0] == 'f') && ( (field2[1] - '0') < 8 && field2[1] >= '0') && ( (field2[4] - 'a') >= 4 ) ) ||
			 (field2[1] == 'b') ||
			 ( (field2[0] == 'f') && ( (field2[1] == 'a') || (field2[1] == 'b') ) ) 
			) 
			{
			 branch_type.call = 1; /* same question why call = 1? */
			 branch_type.branch = 2;
			}
//	fprintf(stderr,"test 5 branch_type.branch = %d\n",branch_type.branch);


		}
	// ARM ARM A4.3 (for further explanation)
	// The following are left for completion, as there may be an easier way to extract r15/pc from 
	// the objdump rather than decoding all instructions of the following types:
	// @todo handle LDR with PC (r15) as destination
	// @todo handle LDM with PC (r15) in register list
	// @todo handle POP with PC (r15) in register list
	// @todo handle arithmetic ops with PC (r15) as destination 
	return;
}
void
arm32_branch_function_old(char* field2)
{
	int instruction_len;
	branch_type.branch = 0;
	branch_type.call = 0;
	branch_type.conditional = 0;
	branch_type.indirect_call = 0;
	branch_type.indirect_jmp = 0;

	instruction_len = strlen(field2);
	fprintf(stderr," encoded instruction length = %d\n",instruction_len);

	// ARM ARM A8.8.18
	// B branches
	// B (T1 encoding -	16-bit [1101|xxxx|xxxx|xxxx] == [d|x|x|x])
	// B (T2 encoding -	16-bit [1110|0xxx|xxxx|xxxx] == [e|<8|x|x])
	// B (T3/T4 encoding - 16+16-bit [1111|0xxx|xxxx|xxxx][10xx|xxxx|xxxx|xxxx])
	// B (A1 encoding -	32-bit [xxxx|1010|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
	if ((field2[0] == 'd') ||
		 ( (field2[0] == 'e' ) && (field2[1] - '0') < 8) ||
		 ( (field2[0] == 'f') && ( (field2[1] -'0') < 8) && ( (field2[4] - '0') == 8 || (field2[4] - '0') == 9)) ||
		 ( (field2[0] == 'f') && ( (field2[1] -'0') < 8) && ( (field2[4] - 'a') == 0 || (field2[4] - 'a') == 1)) ||
		 (field2[1] == 'a')) {
			  // B branches
			  branch_type.branch = 2; /* why 2? */
	}
	fprintf(stderr,"test 1 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.29
	// CBZ/CBNZ compare and branch
	// (T1 encoding - 16-bit [1011|x0x1|xxxx|xxxx])
	if ( ( (field2[0] == 'b' ) && ( ( ((field2[1] - '0') >= 1) && ( (field2[1] - '0') <= 3)) || ( (field2[1] - '0') == 9 ) ) ) ||
	     ( (field2[0] == 'b' ) &&  ( (field2[1] - 'a') == 1) )
		) {
		 // CBZ/CBNZ branches
		 branch_type.branch = 2;
	}
	fprintf(stderr,"test 2 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.27 & A8.8.28    
	// BX/BXJ branch and exchange
	// BX  (T1 encoding - 16-bit [0100|0111|0xxx|xxxx])
	// BXJ (T1 encoding - 16-bit [1111|0011|1100|xxxx])
	// BX  (A1 encoding - 32-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0001|xxxx])
	// BXJ (A1 encoding - 16-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0010|xxxx])
	if (( (field2[0] == '4') && (field2[1] == '7') && ( (field2[3] - '0') < 8 ) ) ||
		 ( (field2[0] == 'f') && (field2[1] == '3') && (field2[3] == 'c')) ||
		 ( (field2[1] == '1') && (field2[3] == '2' ) && ( (field2[6] == '1') || (field2[6] == '2') ) ) ) {
		 // BX/BXJ branches
		 branch_type.branch = 2;
	}
	fprintf(stderr,"test 3 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.236
	// TBB/TBH table branch
	// TBB/TBH (T1 encoding 16+16-bit [1110|1000|1101|xxxx][xxxx|xxxx|000x|xxxx])
	if ( (field2[0] == 'e') && (field2[1] == '8') && (field2[3] == 'd') && ( (field2[6] == '0') ||  (field2[6] == '1') ) ) {
		 branch_type.branch = 2;
	}
	fprintf(stderr,"test 4 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.25
	// BL/BLX calls
	// BL/BLX (T1 encoding 16+16-bit [1111|0xxx|xxxx|xxxx][11x1|xxxx|xxxx|xxxx])
	// BL/BLX (T2 encoding 16+16-bit [1111|0xxx|xxxx|xxxx][11x0|xxxx|xxxx|xxxx])
	// BL  (A1 encoding 32-bit [xxxx|1011|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
	// BLX (A1 encoding 32-bit [1111|101x|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
	if ( ( (field2[0] == 'f') && ( (field2[1] - '0') < 8 && field2[1] >= '0') && ( (field2[4] - 'a') >= 4 ) ) ||
		 (field2[1] == 'b') ||
		 ( (field2[0] == 'f') && ( (field2[1] == 'a') || (field2[1] == 'b') ) ) 
		) {
		 branch_type.call = 1; /* same question why call = 1? */
		 branch_type.branch = 2;
	}
	fprintf(stderr,"test 5 branch_type.branch = %d\n",branch_type.branch);

	// ARM ARM A8.8.26
	// BLX reg (T1 encoding 16-bit [0100|0111|1xxx|xxxx])
	// BLX reg (A1 encoding 32-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0011|xxxx])
	if ((field2[0] == '4' && (field2[1] < '8' && field2[1] >= '0') && ( (field2[2] - '0' == 8) && (field2[2] - '0' == 9)  ) ) ||
	    (field2[0] == '4' && (field2[1] < '8' && field2[1] >= '0') && ( (field2[2] - 'a' >= 0) && (field2[2] - 'a' <= 5)  ) ) ||
		 (field2[1] == '1' && field2[2] == '2' && field2[6] == '3')) {
		 branch_type.call = 1;
		 branch_type.branch = 2;
	}
	fprintf(stderr,"test 6 branch_type.branch = %d\n",branch_type.branch);
	// ARM ARM A4.3 (for further explanation)
	// The following are left for completion, as there may be an easier way to extract r15/pc from 
	// the objdump rather than decoding all instructions of the following types:
	// @todo handle LDR with PC (r15) as destination
	// @todo handle LDM with PC (r15) in register list
	// @todo handle POP with PC (r15) in register list
	// @todo handle arithmetic ops with PC (r15) as destination 
	return;
}
int
arm32_call_identify(char *field2)
{
	int call = 0;

	// ARM ARM A8.8.25
	// BL/BLX calls
	// BL/BLX (T1 encoding 16+16-bit [1111|0xxx|xxxx|xxxx][11x1|xxxx|xxxx|xxxx])
	// BL/BLX (T2 encoding 16+16-bit [1111|0xxx|xxxx|xxxx][11x0|xxxx|xxxx|xxxx])
	// BL  (A1 encoding 32-bit [xxxx|1011|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
	// BLX (A1 encoding 32-bit [1111|101x|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx])
	if ((field2[0] == 'f' && (field2[1] < '8' && field2[1] >= '0') && field2[4] >= 'c') ||
		 (field2[1] == 'b') ||
		 (field2[1] == 'f' && field2[1] >= 'a')) {
		 call = 1; /* same question why call = 1? */
	}

	// ARM ARM A8.8.26
	// BLX reg (T1 encoding 16-bit [0100|0111|1xxx|xxxx])
	// BLX reg (A1 encoding 32-bit [xxxx|0001|0010|xxxx|xxxx|xxxx|0011|xxxx])
	if ((field2[0] == '4' && (field2[1] < '8' && field2[1] >= '0') && field2[2] >= '8') ||
		 (field2[1] == '1' && field2[2] == '2' && field2[6] == '3')) {
		 call = 1;
	}
	return call;
}

void*
x86_branch_function(char* field2)
{
	int branch,base_char;
	uint64_t byte_val;
	char byte_field[3];
	int modrm=0;

	base_char = 0;
	branch = 0;
	byte_val = 0;

	branch_type.branch = 0;
	branch_type.call = 0;
	branch_type.conditional = 0;
	branch_type.indirect_call = 0;
	branch_type.indirect_jmp = 0;

//	test for mod rm
	if(
		((field2[0] == '4') && ((field2[1] - '0') >= 0) && ((field2[base_char+1] - '0') < 10)) ||
		((field2[0] == '4') && ((field2[1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6)) 
			)base_char=2;

		byte_field[0] = field2[base_char+2];
		byte_field[1] = field2[base_char+3];
		byte_field[2] = '\0';
		byte_val = hex_to_ll(byte_field);
		byte_val = (byte_val >>3) & 0x7;

//	test for prefixes
	if(
		((field2[0] == '6') && (field2[1] == '6'))  ||
		((field2[0] == 'f') && (field2[1] == '3'))  ||
		((field2[0] == 'f') && (field2[1] == '2'))  ||
		((field2[0] == 'f') && (field2[1] == '0'))  ||
		((field2[0] == '2') && (field2[1] == 'e'))  ||
		((field2[0] == '3') && (field2[1] == 'e'))  ||
		((field2[0] == '2') && (field2[1] == '6'))  ||
		((field2[0] == '6') && (field2[1] == '4'))  ||
		((field2[0] == '6') && (field2[1] == '5'))  ||
		((field2[0] == '3') && (field2[1] == '6'))  ||
		((field2[0] == '6') && (field2[1] == '7')) 
			)base_char = 2;

	if(
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '0') && ((field2[base_char+3] == '5') 
			|| (field2[base_char+3] == '7'))) ||
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '3') && ((field2[base_char+3] == '4') 
			|| (field2[base_char+3] == '5'))) ||
		((field2[base_char+0] == 'c') && ((field2[base_char+1] == '2') || (field2[base_char+1] == '3'))) ||
		((field2[base_char+0] == 'c') && (((field2[base_char+1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6))) ||
		((field2[base_char+0] == 'e') && 
			((field2[base_char+1] == '0') || (field2[base_char+1] == '1') || 
			(field2[base_char+1] == '2')) ) || 
		((field2[base_char+0] == 'e') && ( (field2[base_char+1] == '9'))) ||
		((field2[base_char+0] == 'e') && ((field2[base_char+1] == 'a') || (field2[base_char+1] == 'b'))) ||
		((field2[base_char+0] == 'f') && (field2[base_char+1] == 'f') && ((byte_val >= 4) && (byte_val <= 5 )))
			) branch_type.branch = 2;

//	conditional
	if(
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '8') && (((field2[base_char+3] - '0') >= 0) 
			&& ((field2[base_char+3] - '0') < 10))) ||
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '8') && (((field2[base_char+3] - 'a') >= 0) 
			&& ((field2[base_char+3] - 'a') < 6))) ||
		((field2[base_char+0] == '7') && (((field2[base_char+1] - '0') >= 0) && ((field2[base_char+1] - '0') < 10))) ||
		((field2[base_char+0] == '7') && (((field2[base_char+1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6))) ||
		((field2[base_char+0] == 'e') && (field2[base_char+1] == '3') ))
			{
			branch_type.branch = 2;
			branch_type.conditional = 1;
			}

//	direct calls
	if(
		(field2[base_char+0] == 'e') && ((field2[base_char+1] == '8')) ||
		((field2[base_char+0] == '9') && (field2[base_char+1] == 'a')) ) 
			{
			branch_type.branch = 2;
			branch_type.call = 1;
			}

//	indirect calls
	if(
		((field2[base_char+0] == 'f') && (field2[base_char+1] == 'f') && ((byte_val >= 2) && (byte_val <= 3 ))) )
			{
			branch_type.branch = 2;
			branch_type.call = 2;
			}
	return;
}

int
x86_branch_identify(char* field2)
{
	int branch,base_char;
	uint64_t byte_val;
	char byte_field[3];
	int modrm=0;

	base_char = 0;
	branch = 0;
	byte_val = 0;

//	test for mod rm
	if(
		((field2[0] == '4') && ((field2[1] - '0') >= 0) && ((field2[base_char+1] - '0') < 10)) ||
		((field2[0] == '4') && ((field2[1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6)) 
			)base_char=2;

		byte_field[0] = field2[base_char+2];
		byte_field[1] = field2[base_char+3];
		byte_field[2] = '\0';
		byte_val = hex_to_ll(byte_field);
		byte_val = (byte_val >>3) & 0x7;

//	test for prefixes
	if(
		((field2[0] == '6') && (field2[1] == '6'))  ||
		((field2[0] == 'f') && (field2[1] == '3'))  ||
		((field2[0] == 'f') && (field2[1] == '2'))  ||
		((field2[0] == 'f') && (field2[1] == '0'))  ||
		((field2[0] == '2') && (field2[1] == 'e'))  ||
		((field2[0] == '3') && (field2[1] == 'e'))  ||
		((field2[0] == '2') && (field2[1] == '6'))  ||
		((field2[0] == '6') && (field2[1] == '4'))  ||
		((field2[0] == '6') && (field2[1] == '5'))  ||
		((field2[0] == '3') && (field2[1] == '6'))  ||
		((field2[0] == '6') && (field2[1] == '7')) 
			)base_char = 2;

	if(
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '0') && ((field2[base_char+3] == '5') 
			|| (field2[base_char+3] == '7'))) ||
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '3') && ((field2[base_char+3] == '4') 
			|| (field2[base_char+3] == '5'))) ||
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '8') && (((field2[base_char+3] - '0') >= 0) 
			&& ((field2[base_char+3] - '0') < 10))) ||
		((field2[base_char+0] == '0') && (field2[base_char+1] == 'f') && (field2[base_char+2] == '8') && (((field2[base_char+3] - 'a') >= 0) 
			&& ((field2[base_char+3] - 'a') < 6))) ||
		((field2[base_char+0] == '7') && (((field2[base_char+1] - '0') >= 0) && ((field2[base_char+1] - '0') < 10))) ||
		((field2[base_char+0] == '7') && (((field2[base_char+1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6))) ||
		((field2[base_char+0] == 'c') && ((field2[base_char+1] == '2') || (field2[base_char+1] == '3'))) ||
		((field2[base_char+0] == 'c') && (((field2[base_char+1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6))) ||
		((field2[base_char+0] == 'e') && 
			((field2[base_char+1] == '0') || (field2[base_char+1] == '1') || 
			(field2[base_char+1] == '2') || (field2[base_char+1] == '3') )) ||
		((field2[base_char+0] == 'e') && ((field2[base_char+1] == '8') || (field2[base_char+1] == '9'))) ||
		((field2[base_char+0] == 'e') && ((field2[base_char+1] == 'a') || (field2[base_char+1] == 'b'))) ||
		((field2[base_char+0] == '9') && (field2[base_char+1] == 'a')) ||
		((field2[base_char+0] == 'f') && (field2[base_char+1] == 'f') && ((byte_val >= 2) && (byte_val <= 5 )))
			) branch = 2;
	return branch;
}

int
x86_call_identify(char* field2)
{
	int call,base_char;
	uint64_t byte_val;
	char byte_field[3];

	base_char = 0;
	call = 0;
	if(
		((field2[0] == '4') && ((field2[1] - '0') >= 0) && ((field2[base_char+1] - '0') < 10)) ||
		((field2[0] == '4') && ((field2[1] - 'a') >= 0) && ((field2[base_char+1] - 'a') < 6)) 
			)base_char=2;
	byte_field[0] = field2[base_char+2];
	byte_field[1] = field2[base_char+3];
	byte_field[2] = '\0';
	byte_val = hex_to_ll(byte_field);
	byte_val = (byte_val >>3) & 0x7;
	if(
		((field2[base_char+0] == 'e') && (field2[base_char+1] == '8')) ||
		((field2[base_char+0] == '9') && (field2[base_char+1] == 'a'))
			) call = 1;
	if(
		((field2[base_char+0] == 'f') && (field2[base_char+1] == 'f') && ((byte_val >= 2) && (byte_val <= 3 )))
			) call = 2;
	
	return call;
}

void
branch_function_init(void)
{
	branch_func_array = (branch_func *) malloc(sizeof(branch_func)*3);
	branch_func_array[0] = (branch_func) &x86_branch_function;
	branch_func_array[1] = (branch_func) &arm32_branch_function;
	branch_func_array[2] = (branch_func) &ppc64_branch_function;
	return;
}

int 
func_asm(pointer_data * global_func_list, int index)
{
	int i,j,k,l,kk,tmp, tmp2, line_count, asm_count,hotspot_index;
	FILE * list, *objout, *dot;
	char spread[]="./spreadsheets", asmd[]="./spreadsheets/asm/", cfg[]="./spreadsheets/cfg/", src[]="./spreadsheets/src/";
	char  sheetname[] = "_asm.csv", cfg_name[] = "_cfg.dot", obj1[] = " -d --start-address=0x", obj2[] = " --stop-address=0x";
	char svg_name[] = "_cfg.svg", *svg_cmd, *svg_file;
	char* spreadsheet,* cfg_file;
	char null_string[] = " null";
	int null_string_len=5, filename_len, asmd_len, cfg_len, cfg_name_len;
	char * funcname, *filename, *obj_cmd, line_buf[4096];
	int good_line;
	char mode[] = "w+";
	char field1[4096], field2[4096],field3[4096],target_address[80],byte_field[3];
	int base_char, field2_len,field3_len,text_len,bb_text_len;
	size_t funcname_len, spreadsheet_len = 20, line_buf_len = 4096, buf_len, obj1_len = 29, obj2_len = 18, module_len, asm_len;
	function_struc_ptr this_function;
	module_struc_ptr this_module;
	process_struc_ptr this_process;
	sample_struc_ptr loop_rva;
	asm_struc_ptr this_asm=NULL, next_asm=NULL, previous_asm=NULL, loop_asm=NULL, old_loop_asm;
	basic_block_struc_ptr this_bb=NULL, next_bb=NULL, previous_bb=NULL, loop_bb=NULL, target_bb, last_bb_struc;
	int *sample_count, asm_total_sample_count, asm_cycle_count;
	float summed_samples, total_samples;
	size_t base, end;
	int count, branch, branch_count, call, first_bb, last_bb, bb_count, deadbeef, first_src_bb;
	uint64_t address, old_address, end_address, *branch_address,byte_val, first_asm=0, last_asm, last_bb_end, this_bb_end;
	const char * source_file, *source_file_old;
	int src_file_path_len, ret_val, inline_loop_count;
	unsigned int line_nr, line_nr_old;
	file_list_struc_ptr principal_file_loop,old_principal_file,principal_file_max=NULL;
	int max_principal_count = -1, num_col, src_file_test = 0;
	int min_file_line, max_file_line, source_line_count, *source_sample_count, sample_count_max, sample_count_line;
	addr_list_data * bb_addr_list;
	float color_index;
	int fillcolor,max_bb_count=0;

	typedef struct next_taken_struc* next_taken_struc_ptr;
	typedef struct next_taken_struc{
		next_taken_struc_ptr	next;
		next_taken_struc_ptr	previous;
		uint64_t		address;
		uint64_t		count;
		}next_taken_data;
	uint64_t next_taken_count_sum;
	next_taken_struc_ptr next_taken_stack, next_taken_stack_loop, previous_stack, this_next_taken;
	branch_struc_ptr next_taken_loop;
	uint64_t next_taken_address;
	int valid_branch_target=0;

	spreadsheet_len = strlen(sheetname);
	asmd_len = strlen(asmd);
	cfg_name_len = strlen(cfg_name);
	cfg_len = strlen(cfg);
	total_samples = global_sample_count_in_func;
	summed_samples = 0;
	asm_total_sample_count = 0;
	asm_cycle_count = 0;
	next_taken_count_sum = 0;
	next_taken_stack = NULL;

	i = index;
	hotspot_index = global_func_count - 1 - index;

	max_file_line = 0;
	min_file_line = 1000000;
	bb_text_len = 100;

	branch_count = 0;
	deadbeef = 0;
	this_asm = NULL;
	previous_asm = NULL;
	this_bb = NULL;
	previous_bb = NULL;
	last_bb_end = 0;
		
	this_function = (function_struc_ptr) global_func_list[i].ptr;
	count = global_func_list[i].val;
	this_module = this_function->this_module;
	this_process = this_function->this_process;
	loop_rva = this_function->first_rva;
#ifdef DBUG
	fprintf(stderr," func_asm index = %d, %d, function %s, first rva = 0x%"PRIx64", base = 0x%"PRIx64", len = %lx, total sample count = %d\n",i,hotspot_index,this_function->function_name, loop_rva->rva,this_function->function_rva_start, this_function->function_length, this_function->total_sample_count);
	fprintf(stderr," asmd_len = %d, asmd = %s\n",asmd_len,asmd);
#endif
//	create asm spreadsheet file name string

	funcname_len = strlen(this_function->function_name);
	filename = (char*) malloc((20 + asmd_len + spreadsheet_len)*sizeof(char));
	for(j=0; j < asmd_len; j++)filename[j]=asmd[j];
	sprintf(&filename[asmd_len],"%d\0",hotspot_index);
	filename_len = strlen(filename);
#ifdef DBUG
	fprintf(stderr," %d len = %d, filename = %s\n",i,filename_len, filename);
#endif
	for(j=0; j< spreadsheet_len; j++)filename[filename_len+j] = sheetname[j];
	filename[filename_len + spreadsheet_len] = '\0';
#ifdef DBUG
	fprintf(stderr," spreadsheet filename for file %d = %s\n",i,filename);
#endif
	list = fopen(filename,mode);
	if(list == NULL)
		{
		fprintf(stderr,"failed to open file %s\n",filename);
		err(1,"failed to open asm listing file");
		}
//	create cfg file name string
	cfg_file = (char*)malloc(20 + cfg_len + cfg_name_len);
	sprintf(cfg_file,"%s%d%s\0",cfg,hotspot_index,cfg_name);
#ifdef DBUG
	fprintf(stderr," %d cfg filename = %s\n",i,cfg_file);
#endif
	dot = fopen(cfg_file,mode);
	if(dot == NULL)
		{
		fprintf(stderr,"failed to open file %s\n",cfg_file);
		err(1,"failed to open asm cfg file");
		}
//	printf first few rows of cfg file
	fprintf(dot,"digraph \"%s\"{\n",this_function->function_name);
	fprintf(dot,"\trankdir=LR;\n");
	fprintf(dot,"\tnode[shape=box,colorscheme=ylorrd9, style=filled];\n\n");
		
	num_col = global_event_order->num_fixed + global_event_order->num_ordered;

//	check the old_module_path and set up asm_2_src as needed

	if((old_module_path == NULL) || (old_module_path != this_module->local_path))
		{
		if(old_module_path != NULL)asm_2_src_close();
		asm_2_src_status = asm_2_src_init(this_module->local_path);
		old_module_path = this_module->local_path;
		}
#ifdef DBUG
	fprintf(stderr," module local_path = %s, asm_2_src_status = %d\n",this_module->local_path,asm_2_src_status);
#endif
//	create the objdump command for this module and function
	module_len = strlen(this_function->this_module->local_path);
	obj_cmd = (char*)malloc((objdump_len + obj1_len + 16 + obj2_len + 16 + module_len +1)*sizeof(char));
	if(obj_cmd == NULL)
		{
		fprintf(stderr,"failed to malloc obj_cmd for %s, %s\n",filename, this_function->this_module->local_path);
		err(1,"failed to malloc obj_cmd space");
		}
	base = this_function->function_rva_start;
	end = base + this_function->function_length - 1;
	sprintf(obj_cmd,"%s%s%"PRIx64"%s%"PRIx64" %s\0",objdump_bin,obj1,base,obj2,end,this_function->this_module->local_path);
#ifdef DBUG
	fprintf(stderr," obj command = %s\n",obj_cmd);
#endif
	objout = popen(obj_cmd, "r");
	line_count = 0;
	asm_count = 0;
	while(fgets(line_buf,line_buf_len,objout) != NULL)
		{
		buf_len = strlen(line_buf);
		line_count++;
		good_line = 0;
#ifdef DBUG
		fprintf(stderr," objdump -d len = %zu, line = %s",buf_len,line_buf);
#endif
		if((buf_len + 5) > line_buf_len)
			fprintf(stderr," very long objdump output len = %zu, line = %s",buf_len,line_buf);
//		test first few characters to find first line of asm
//		the following only works with "small" addresses
//		full 64 bit addresses end up left justified in objdump output
//		if(line_buf[0] != ' ')continue;


		if( (line_buf[0] == ' ') || 
			(  ((line_buf[0]-'0') >= 0) && ((line_buf[0]-'0') <= 9) ) ||
			(  ((line_buf[0]-'a') >= 0) && ((line_buf[0]-'a') <= 7) ))
				good_line = 1; 
		if(good_line != 1)continue;

//		fprintf(stderr," objdump -d len = %zu, line = %s",buf_len,line_buf);
#ifdef DBUG
		fprintf(stderr," objdump -d len = %zu, line = %s",buf_len,line_buf);
#endif
		asm_count++;
//	first field is the address
		j = 0;
		while((line_buf[j] != ':') && (j < buf_len))
			{
			field1[j] = line_buf[j];
			if(line_buf[j] == '<')good_line = 0;
			j++;
			}
		if(good_line != 1)continue;
		field1[j] = '\0';
		j++;
		field1[j] = '\0';
		if(line_buf[j] != '\t')
			{
			fprintf(stderr," lost after first field, j = %d, line = %s\n",j,line_buf);
			err(1,"got lost decoding asm_line");
			}
		j++;
#ifdef DBUG
			fprintf(stderr," first field = %s\n",field1);
#endif
//	second field is the encoding
		k=0;
//			while(((line_buf[j] != '\n') || (line_buf[j] != '\t')) && (j < buf_len))
		while( (line_buf[j] != '\t') && (j < buf_len))
			{
			if(line_buf[j] != ' ')
				{
				field2[k] = line_buf[j];
				k++;
				}
			j++;
			}
		field2[k] = '\0';
		field2_len = strlen(field2);
#ifdef DBUG
			fprintf(stderr," second field = %s\n",field2);
#endif
//	third field is the asm
		k = 0;
		branch = 0;
		call = 0;
		while( j < buf_len ) 
			{
			if(line_buf[j] == '<')
				{
				branch = 1;
				branch_count++;
				break;
				}
			if(line_buf[j] == '#')break;

			field3[k] = line_buf[j];
			k++;
			j++;
			}
		field3[k] = '\0';
		if(j == buf_len)field3[k-1] = '\0';
		field3_len = strlen(field3);
		if(k==0)
			{
			field3_len = 0;
			for(k=0;k<null_string_len;k++)field3[k] = null_string[k];
			k=null_string_len;
			}
		field3[k] = '\0';

		if(field1[0] == ' ')
			{
			address = hex_to_ll(&field1[1]);
			}
		else
			{
			address = hex_to_ll(&field1[0]);
			}
		if(address == BAD)
			{
			fprintf(stderr," bad address returned from hex_to_ll for field1 = %s, at 0x%p\n",field1, field1);
			err(1, "bad hex_to_ll");
			}
#ifdef DBUG
		fprintf(stderr," line address = 0x%"PRIx64", field3 = %s\n",address, field3);
		if(loop_rva == NULL)
			{
			fprintf(stderr," address = 0x%"PRIx64", and loop_rva = %p\n",address,loop_rva);
			}
		else
			{
			fprintf(stderr,"1 address = 0x%"PRIx64", and loop_rva->rva = 0x%"PRIx64"\n",address,loop_rva->rva);
			}
#endif
		if(first_asm == 0)first_asm = address;
		last_asm = address;
//		if(loop_rva != NULL)
//			{
//			while((address > loop_rva->rva) && (loop_rva != NULL))
//				{
//				loop_rva=loop_rva->next;
//				if(loop_rva == NULL)break;
//#ifdef DBUG
//      follow a single address through if there are worries about lost samples/rva struc's
//		if(loop_rva->rva == 0x43e110)fprintf(stderr,"func_asm 0 function for address 0x43e110, cycle count = %d\n",
//				loop_rva->sample_count[num_events*(num_cores + num_sockets)]);
//		if(loop_rva->rva == 0x43e111)fprintf(stderr,"func_asm 0 for address 0x43e111, cycle count = %d\n",
//				loop_rva->sample_count[num_events*(num_cores + num_sockets)]);
//#endif
//
//#ifdef DBUG
//				if(loop_rva == NULL)
//					{
//					fprintf(stderr," address = 0x%"PRIx64", and loop_rva = %p\n",address,loop_rva);
//					}
//				else
//					{
//					fprintf(stderr,"2 address = 0x%"PRIx64", and loop_rva->rva = 0x%"PRIx64"\n",address,loop_rva->rva);
//					}
//#endif
//				}
//			}
//	check for branch instructions that do not place the target address in the asm text
//	placed off in function to allow other architectures to be used.
//	This appears to be the only ISA dependent component of the analysis
		if((field3_len != 0))
			branch_func_array[arch_type_flag] (field2);
//			x86_branch_function(field2);
#ifdef DBUG
		fprintf(stderr," branch_func: field3_len = %d, field2 = %s, branch = %d, branch_type.branch = %d, branch_type.call = %d, branch_type.conditional = %d\n",
			field3_len,field2,branch,branch_type.branch,branch_type.call,branch_type.conditional);
#endif
		if(branch == 1)branch_type.branch=1;
		branch = branch_type.branch;
		call = branch_type.call;


//		build asm struc
//
//#ifdef DBUG
//		if(loop_rva == NULL)
//			{
//			fprintf(stderr," address = 0x%"PRIx64", and loop_rva = %p\n",address,loop_rva);
//			}
//		else
//			{
//			fprintf(stderr,"3 address = 0x%"PRIx64", and loop_rva->rva = 0x%"PRIx64"\n",address,loop_rva->rva);
//			}
//#endif

		this_asm = asm_struc_create();
		if(this_asm == NULL)
			{
			fprintf(stderr,"failed to create asm struc for function %s, line %s\n",this_function->function_name,line_buf);
			err(1,"failed to create asm struc");
			}
		if(this_function->first_asm == NULL) this_function->first_asm = this_asm;
		if(previous_asm != NULL)previous_asm->next = this_asm;
		this_asm->previous = previous_asm;
		this_asm->address = address;
		this_asm->branch = branch;
		this_asm->call = call;
		this_asm->conditional = branch_type.conditional;
#ifdef DBUG
		if(call != 0)
			fprintf(stderr,"call nonzero %d at address 0x%"PRIx64"\n",call,address);
#endif
		this_asm->encoding = (char*)malloc(field2_len+1);
		for(kk=0; kk<field2_len; kk++)this_asm->encoding[kk]=field2[kk];
		this_asm->encoding[field2_len] = '\0';
#ifdef DBUG
		fprintf(stderr,"this_asm struc has components address = 0x%"PRIx64", and branch = %d\n",this_asm->address, this_asm->branch);
#endif

//		insert asm_2_src code here
		if(asm_2_src_status == 0)
			{
			ret_val = asm_2_src(this_asm->address, &source_file, &line_nr);
#ifdef DBUG
			fprintf(stderr,"returned from asm_2_src\n");
			fprintf(stderr," asm_2_src retval = %d, source file = %p, line_nr = %d\n",ret_val,source_file,line_nr);
			fprintf(stderr," asm_2_src retval = %d, source file = %s, line_nr = %d\n",ret_val,source_file,line_nr);
#endif
			src_file_test = 0;
			if(source_file != NULL)src_file_test = strcmp(source_file,old_module_path);
			if(ret_val == 1)
				{
				if((source_file != NULL) && (src_file_test != 0))
					{
					src_file_path_len = strlen(source_file);
					this_asm->initial_source_file = (char *)malloc(src_file_path_len+1);
					if(this_asm->initial_source_file == NULL)
						{
						fprintf(stderr,"malloc failed for initial_asm_source_file, func = %s\n",this_function->function_name);
						err(1,"failed malloc for initial_asm_source_file");
						}
					k = 0;
					for(kk=0;kk<src_file_path_len; kk++)
						{
						this_asm->initial_source_file[kk] = source_file[kk];
						if(source_file[kk] == '/')k=kk+1;
						}
					this_asm->initial_source_file[src_file_path_len] = '\0';
					this_asm->initial_source_line = line_nr;
					this_asm->initial_source_name = (char *)malloc(src_file_path_len+2 - k);
					if(this_asm->initial_source_name == NULL)
						{
						fprintf(stderr,"malloc failed for initial_asm_source_name, func = %s\n",this_function->function_name);
						err(1,"failed malloc for initial_asm_source_name");
						}
					for(kk=k; kk<src_file_path_len; kk++)this_asm->initial_source_name[kk - k] = source_file[kk];
					this_asm->initial_source_name[src_file_path_len - k] = '\0';
					}
				else
					{
					this_asm->initial_source_file = (char *)malloc(5);
					if(this_asm->initial_source_file == NULL)
						{
						fprintf(stderr,"malloc failed for NULL initial_asm_source_file, func = %s\n",this_function->function_name);
						err(1,"failed malloc for NULL initial_asm_source_file");
						}
					this_asm->initial_source_name = (char *)malloc(5);
					if(this_asm->initial_source_name == NULL)
						{
						fprintf(stderr,"malloc failed for NULL initial_asm_source_name, func = %s\n",this_function->function_name);
						err(1,"failed malloc for NULL initial_asm_source_name");
						}
					this_asm->initial_source_file[0] = 'n';
					this_asm->initial_source_file[1] = 'u';
					this_asm->initial_source_file[2] = 'l';
					this_asm->initial_source_file[3] = 'l';
					this_asm->initial_source_file[4] = '\0';
					this_asm->initial_source_name[0] = 'n';
					this_asm->initial_source_name[1] = 'u';
					this_asm->initial_source_name[2] = 'l';
					this_asm->initial_source_name[3] = 'l';
					this_asm->initial_source_name[4] = '\0';
					}
#ifdef DBUG
				fprintf(stderr," address = 0x%"PRIx64", path = %s, name = %s\n",this_asm->address,this_asm->initial_source_file,this_asm->initial_source_name);
#endif
				}
			k = 0;
			source_file_old = source_file;
			line_nr_old = line_nr;
			if((source_file_old != NULL) && (src_file_test != 0))
				{
				ret_val = 1;
				inline_loop_count = 0;
				while(ret_val != 0)
					{
#ifdef DBUG
					fprintf(stderr,"about to call asm_2_src_inline\n");
#endif
					ret_val = asm_2_src_inline( &source_file, &line_nr);
					inline_loop_count++;
					if(inline_loop_count > 20)
						{
						ret_val =0;
						fprintf(stderr,"inline depth too large in function %s for address 0x%"PRIx64"\n",this_function->function_name,this_asm->address);
						}
#ifdef DBUG
					fprintf(stderr,"returned from asm_2_src_inline\n");
					fprintf(stderr," asm_2_src_inline retval = %d, source file = %p, line_nr = %d\n",ret_val,source_file,line_nr);
					fprintf(stderr," asm_2_src_inline retval = %d, source file = %s, line_nr = %d\n",ret_val,source_file,line_nr);
#endif
					if(ret_val == 0)
						{
						source_file_old = source_file;
						line_nr_old = line_nr;
						}
					}
				src_file_test = 0;
				if(source_file != NULL)src_file_test = strcmp(source_file,old_module_path);
				}
			if((source_file_old != NULL) && (src_file_test != 0))
				{
				src_file_path_len = strlen(source_file_old);
				this_asm->principal_source_file = (char *)malloc(src_file_path_len+1);
				if(this_asm->principal_source_file == NULL)
					{
					fprintf(stderr,"malloc failed for principal_asm_source_file, func = %s\n",this_function->function_name);
					err(1,"failed malloc for principal_asm_source_file");
					}
				k = 0;
				for(kk=0;kk<src_file_path_len; kk++)
					{
					this_asm->principal_source_file[kk] = source_file_old[kk];
					if(source_file_old[kk] == '/')k=kk + 1;
					}
				this_asm->principal_source_file[src_file_path_len] = '\0';
				this_asm->principal_source_line = line_nr_old;
				if(line_nr_old < min_file_line)min_file_line = line_nr_old;
				if(line_nr_old > max_file_line)max_file_line = line_nr_old;
				this_asm->principal_source_name = (char *)malloc(src_file_path_len+1 - k);
				if(this_asm->principal_source_name == NULL)
					{
					fprintf(stderr,"malloc failed for principal_asm_source_name, func = %s\n",this_function->function_name);
					err(1,"failed malloc for principal_asm_source_name");
					}
				for(kk=k; kk<src_file_path_len; kk++)this_asm->principal_source_name[kk - k] = source_file_old[kk];
				this_asm->principal_source_name[src_file_path_len - k] = '\0';
				}
			else
				{
				this_asm->principal_source_file = (char *)malloc(5);
				if(this_asm->principal_source_file == NULL)
					{
					fprintf(stderr,"malloc failed for NULL principal_asm_source_file, func = %s\n",this_function->function_name);
					err(1,"failed malloc for NULL principal_asm_source_file");
					}
				this_asm->principal_source_name = (char *)malloc(5);
				if(this_asm->principal_source_name == NULL)
					{
					fprintf(stderr,"malloc failed for NULL principal_asm_source_name, func = %s\n",this_function->function_name);
					err(1,"failed malloc for NULL principal_asm_source_name");
					}
				this_asm->principal_source_file[0] = 'n';
				this_asm->principal_source_file[1] = 'u';
				this_asm->principal_source_file[2] = 'l';
				this_asm->principal_source_file[3] = 'l';
				this_asm->principal_source_file[4] = '\0';
				this_asm->principal_source_name[0] = 'n';
				this_asm->principal_source_name[1] = 'u';
				this_asm->principal_source_name[2] = 'l';
				this_asm->principal_source_name[3] = 'l';
				this_asm->principal_source_name[4] = '\0';
				}
#ifdef DBUG
			fprintf(stderr," address = 0x%"PRIx64", path = %s, name = %s\n",this_asm->address,this_asm->principal_source_file,this_asm->principal_source_name);
#endif
//	find this file in the function principal file list and increment the count
			principal_file_loop = this_function->principal_file_list;
			old_principal_file = NULL;
			while(principal_file_loop != NULL)
				{
				if(strcmp(principal_file_loop->principal_source_file,this_asm->principal_source_file) == 0)break;
				old_principal_file = principal_file_loop;
				principal_file_loop = principal_file_loop->next;
				}
			if(principal_file_loop == NULL)
				{
				principal_file_loop = file_list_create();
				if(principal_file_loop == NULL)
					{
					fprintf(stderr,"failed to create file_list_struc for function %s\n",this_function->function_name);
					err(1,"failed to malloc file_list");
					}
				src_file_path_len=strlen(this_asm->principal_source_file);
				principal_file_loop->principal_source_file = malloc(src_file_path_len+1);
				if(principal_file_loop->principal_source_file == NULL)
					{
					fprintf(stderr,"failed to malloc principal source name for function %s\n",this_function->function_name);
					err(1,"failed to malloc file_list");
					}
				for(kk=0;kk<src_file_path_len; kk++)principal_file_loop->principal_source_file[kk] = this_asm->principal_source_file[kk];
				principal_file_loop->principal_source_file[src_file_path_len] = '\0';
				src_file_path_len=strlen(this_asm->principal_source_name);
				principal_file_loop->principal_source_name = malloc(src_file_path_len+1);
				if(principal_file_loop->principal_source_name == NULL)
					{
					fprintf(stderr,"failed to malloc principal source name for function %s\n",this_function->function_name);
					err(1,"failed to malloc file_list");
					}
				for(kk=0;kk<src_file_path_len; kk++)principal_file_loop->principal_source_name[kk] = this_asm->principal_source_name[kk];
				principal_file_loop->principal_source_name[src_file_path_len] = '\0';
				if(old_principal_file == NULL)
					{
					this_function->principal_file_list = principal_file_loop;
					}
				else
					{
					old_principal_file->next = principal_file_loop;
					principal_file_loop->previous = old_principal_file;
					}
				}
			principal_file_loop->count++;
			}
								

		asm_len = strlen(field3);
		this_asm->asm_text = (char *) malloc((asm_len+2)*sizeof(char));
		if(this_asm->asm_text == NULL)
			{
			fprintf(stderr,"failed to create asm string for function %s, line %s\n",this_function->function_name,line_buf);
			err(1,"failed to create asm string");
			}
		for(k=0; k< asm_len; k++)this_asm->asm_text[k] = field3[k];
		this_asm->asm_text[asm_len] = ' ';
		this_asm->asm_text[asm_len+1] = '\0';
#ifdef DBUG
		fprintf(stderr," asm = %s\n",this_asm->asm_text);
#endif
//		copy rva sample_count, for now just the totals
		if(loop_rva != NULL)
			{
			while( (loop_rva != NULL) && (address >= loop_rva->rva))
				{
#ifdef DBUG
//      follow a single address through if there are worries about lost samples/rva struc's
//		if(loop_rva->rva == 0x43e110)fprintf(stderr,"func_asm 1 function for address 0x43e110, cycle count = %d\n",
//				loop_rva->sample_count[num_events*(num_cores + num_sockets)]);
//		if(loop_rva->rva == 0x43e111)fprintf(stderr,"func_asm 1 for address 0x43e111, cycle count = %d\n",
//				loop_rva->sample_count[num_events*(num_cores + num_sockets)]);
#endif
				this_asm->total_sample_count += loop_rva->total_sample_count;
				asm_cycle_count += this_asm->sample_count[num_events*(num_cores + num_sockets)];
				for(k=0;k<num_events; k++)
					{
					this_asm->sample_count[num_events*(num_cores + num_sockets) + k] += 
						loop_rva->sample_count[num_events*(num_cores + num_sockets) + k];
					asm_total_sample_count += loop_rva->sample_count[num_events*(num_cores + num_sockets) + k];
					}
				if(source_index != 0)
					this_asm->sample_count[source_index] += loop_rva->sample_count[source_index];
				if(target_index != 0)
					this_asm->sample_count[target_index] += loop_rva->sample_count[target_index];
				if(next_taken_index != 0)
					this_asm->sample_count[next_taken_index] += loop_rva->sample_count[next_taken_index];
#ifdef DBUG
		if(strcmp(this_function->function_name, "context_switch.isra.59") == 0) 
				printf_rva(loop_rva, this_function, this_function->this_process);
#endif
//				this structure should be overwritten by the last rva..which is the closest to the address
				this_asm->next_taken_list = loop_rva->next_taken_list;

				loop_rva = loop_rva->next;
				}
			}
		if(branch == 1)
			{
//		branch instruction, get address of target
//		fprintf(stderr," field3 = %s\n",field3);
#ifdef DBUG
			fprintf(stderr," field3_len = %d,field3 = %s\n",field3_len,field3);
#endif
			j = 0;
			if(field3[j] == '\t')j++;
			if(field3[j] == ' ')j++;
//		fixup for lock prefix on x86 branch...only seen once
			if( (field3[j] == 'l') &&(field3[j+1] == 'o') && (field3[j+2] == 'c')) j+=5;
			while((field3[j] != ' ') && (field3[j] != '\t'))j++;
#ifdef DBUG
			fprintf(stderr," j after while loop = %d, %c%c\n",j,field3[j],field3[j+1]);
#endif
			if(field3[j] == '\t')j++;
			while(field3[j] == ' ')j++;
//		fixup for PPC64
			if(field3[j+1] == 'r')j+=4;
			
//				start of target address
			k = 0;
#ifdef DBUG
			fprintf(stderr," starting index of target = %d, %c%c\n",j,field3[j],field3[j+1]);
#endif
			while((field3[j] != ' ') && (field3[j] != '\0'))
				{
				target_address[k] = field3[j];
				j++;
				k++;
				}
			target_address[k] = '\0';
#ifdef DBUG
			fprintf(stderr," address =  %s, len = %ld, first char = %c\n", target_address,strlen(target_address),target_address[0]);
#endif

			this_asm->target = hex_to_ll(target_address);

#ifdef DBUG
			fprintf(stderr," address = 0x%"PRIx64", %s\n",this_asm->target, target_address);
#endif
			if(this_asm->target == BAD)
				{
				fprintf(stderr," bad address returned from hex_to_ll for target_address = %s, at 0x%p\n",target_address,target_address);
				err(1, "bad hex_to_ll");
				}
			}
		if(branch == 2)
			{
			deadbeef++;
			this_asm->target = 0xdeadbeef000 + deadbeef;
			branch_count++;
			}
		previous_asm = this_asm;
//		if((source_file_old != source_file) && (source_file_old != NULL))free((void *)source_file_old);
//		if(source_file != NULL)free((void *)source_file);
		}
//	end of loop over objdump output

#ifdef DBUG
	fprintf(stderr," finished loop over line_buf, branch_count = %d, filename = %s\n", branch_count,this_function->function_name);
	fprintf(stderr," function =%s, function_sample_count = %d, function cycle count = %d, asm_sample_count = %d, asm_cycle_count = %d\n",
		this_function->function_name, this_function->total_sample_count, this_function->cycle_count, 
			asm_total_sample_count, asm_cycle_count);
	loop_asm = this_function->first_asm;
	while(loop_asm != NULL)
		{
		fprintf(stderr,"address = 0x%"PRIx64"  %s   %s   branch = %d, target = 0x%"PRIx64"\n",
			loop_asm->address,loop_asm->encoding,loop_asm->asm_text, loop_asm->branch, loop_asm->target);
		loop_asm = loop_asm->next;
		}
#endif
//identify the principal file for the function and create its structure

	principal_file_loop = this_function->principal_file_list;
	max_principal_count = 0;
	principal_file_max = NULL;
	while(principal_file_loop != NULL)
		{
		if(principal_file_loop->count > max_principal_count)
			{
			max_principal_count = principal_file_loop->count;
			principal_file_max = principal_file_loop;
			}
#ifdef DBUG
		fprintf(stderr,"principal file path = %s, count = %d\n",principal_file_loop->principal_source_file,principal_file_loop->count);
		fprintf(stderr,"principal file name = %s, count = %d\n",principal_file_loop->principal_source_name,principal_file_loop->count);
#endif
		principal_file_loop = principal_file_loop->next;
		}
	this_function->principal_file = principal_file_struc_create();
	if(this_function->principal_file == NULL)
		{
		fprintf(stderr," could not create principal_source_file struc for %s\n",this_function->function_name);
		err(1, "could not create principal struc");
		}
	if(principal_file_max != NULL)
		{
		this_function->principal_file->principal_file_name = principal_file_max->principal_source_name;
		this_function->principal_file->path = principal_file_max->principal_source_file;
		}
#ifdef DBUG
	fprintf(stderr,"this_function->principal file path = %s\n",this_function->principal_file->path);
	fprintf(stderr,"this_function->principal file name = %s\n",this_function->principal_file->principal_file_name);
#endif

//		create the address list
	branch_address = (uint64_t *)malloc(2*(branch_count+2)*sizeof(uint64_t));
	if(branch_address == NULL)
		{
		fprintf(stderr," could not malloc branch_address for %s\n",this_function->function_name);
		err(1, "could not malloc branch_address");
		}
#ifdef DBUG
	fprintf(stderr," number of elements in branch_address = %d\n",2*(branch_count+2));
#endif
	j = 0;
	loop_asm = this_function->first_asm;
	branch_address[0] = loop_asm->address;
	if(last_asm < end)
		{
		if((end-last_asm) > 1)
			fprintf(stderr," readelf inconsistent with objdump in function %s, end = 0x%"PRIx64", last_asm = 0x%"PRIx64"\n",
			this_function->function_name,end,last_asm);
		end = last_asm;
		}
	while(j<2*branch_count+1)
		{
		loop_asm = loop_asm->next;
		if(loop_asm == NULL)
			{
			break;
			}
		if(loop_asm->branch >= 1)
			{
			if(loop_asm->next != NULL)
				{
				branch_address[j+1] = loop_asm->next->address;
				}
			else
				{
				branch_address[j+1] = end + 1;
				}
			branch_address[j+2] = loop_asm->target;
			j+=2;
#ifdef DBUG
			if(j > 2*branch_count)fprintf(stderr,"j > 2*branch_count, j = %d\n",j);
#endif
			}
		}
	branch_address[j+1] = end+1;
#ifdef DBUG
	fprintf(stderr,"first branch = 0x%"PRIx64"\n",branch_address[0]);
	fprintf(stderr," last address = 0x%"PRIx64", j = %d\n",branch_address[j],j);
	fprintf(stderr," calling quicksort64 with %d addresses\n",(j+1) );
#endif
	quickSort64(branch_address, j+2);
#ifdef DBUG
	fprintf(stderr," base address = 0x%"PRIx64", end = 0x%"PRIx64"\n",base,end);
	fprintf(stderr,"first branch = 0x%"PRIx64"\n",branch_address[0]);
	k=0;
	while((branch_address[k] <= end+1) && k < (2*(branch_count+2) ))
		{
		k++;
//		fprintf(stderr," k = %d and number of elements in branch_address = %d\n",k,2*(branch_count+2) );
		}
	fprintf(stderr," last address for end = %d, = 0x%"PRIx64"\n",k,branch_address[k]);
	fprintf(stderr," last address = 0x%"PRIx64"\n",branch_address[j+1]);
	fprintf(stderr," end address = 0x%"PRIx64"\n",end);
#endif
	last_bb = 0;
	first_bb = 0;
	while(branch_address[first_bb] < base)first_bb++;
#ifdef DBUG
	fprintf(stderr,"address of first_bb = 0x%"PRIx64", first bb = %d\n",branch_address[first_bb], first_bb);
#endif
	while((branch_address[last_bb] <=  end) && (last_bb < j+1))last_bb++;
	last_bb --;
	if(last_bb == 0)last_bb = 1;
#ifdef DBUG
	fprintf(stderr,"last bb = %d\n",last_bb);
#endif

//		create the basic block structures

//	create source line numbering array for BB dominant source line
	if(max_file_line > 0)
		{
		source_line_count = max_file_line - min_file_line + 1;
		source_sample_count = (int*)malloc(source_line_count*sizeof(int));
		if(source_sample_count == NULL)
			{
			fprintf(stderr,"failed to malloc sample count buffer for BB analysis\n");
			err(1,"malloc failed for sample counts for BB");
			}
		}
//	create the BBs
	k = first_bb;
	old_address = branch_address[k];
	loop_asm = this_function->first_asm;
	bb_count = 0;
	deadbeef = 0;
	previous_bb = NULL;
	while(k < last_bb)
		{
		if((branch_address[k] == old_address) && (k != first_bb))
			{
			k++;
			continue;
			}
		this_bb = basic_block_struc_create();
		if(this_bb == NULL)
			{
			fprintf(stderr,"failed to create BB struc for function %s, address 0x%"PRIx64", entry %d\n",this_function->function_name,branch_address[k], k);
			err(1,"failed to create BB struc");
			}
		last_bb_struc = this_bb;
		if(this_function->first_bb == NULL) this_function->first_bb = this_bb;
		if(previous_bb != NULL)
			{
			previous_bb->next = this_bb;
//			fprintf(stderr,"setting next for the previous_bb = %d, address = 0x%"PRIx64"\n",previous_bb->block_count, previous_bb->address);
			}
		this_bb->previous = previous_bb;
		this_bb->address = branch_address[k];
//		set end adderess to end as a safety measure
		this_bb->end_address = end;
		this_bb->block_count = bb_count+1;
#ifdef DBUG
		fprintf(stderr,"bb count = %d, address = 0x%"PRIx64"\n",bb_count, this_bb->address);
#endif
		this_bb->text = (char*)malloc(bb_text_len);
		if(this_bb->text == NULL)
			{
			fprintf(stderr,"failed to create BB struc text for function %s, address 0x%"PRIx64", entry %d\n",this_function->function_name,branch_address[k], k);
			err(1,"failed to create BB struc text");
			}
		tmp = k;
		if(max_file_line > 0)
			{
			for(tmp2=0; tmp2<source_line_count; tmp2++)source_sample_count[tmp2]=0;
			sample_count_max = 0;
			}

		while((tmp < last_bb) && (branch_address[tmp] == branch_address[k]))tmp++;
#ifdef DBUG
		fprintf(stderr,"bb count = %d, address = 0x%"PRIx64", tmp = %d, branch_address = 0x%"PRIx64"\n",
			bb_count, this_bb->address, tmp, branch_address[tmp]);
		if(tmp == last_bb -1)fprintf(stderr," tmp = %d equals last_bb - 1, loop_asm->address = 0x%"PRIx64", end = 0x%"PRIx64"\n",
				tmp, loop_asm->address, end);
		if(tmp == last_bb )fprintf(stderr," tmp = %d equals last_bb , loop_asm->address = 0x%"PRIx64", end = 0x%"PRIx64"\n",
				tmp, loop_asm->address, end);
#endif
		this_bb_end = branch_address[tmp];
		if(tmp == last_bb)this_bb_end = end + 1;
		this_bb->source_line = loop_asm->principal_source_line;
		while(loop_asm->address < this_bb_end)
			{
			for(tmp2=0; tmp2 < num_events; tmp2++)
				this_bb->sample_count[num_events*(num_cores + num_sockets) + tmp2] +=
					loop_asm->sample_count[num_events*(num_cores + num_sockets) + tmp2];
			if(source_index != 0)
				this_bb->sample_count[source_index] += loop_asm->sample_count[source_index];
			if(target_index != 0)
				this_bb->sample_count[target_index] += loop_asm->sample_count[target_index];
			if(next_taken_index != 0)
				this_bb->sample_count[next_taken_index] += loop_asm->sample_count[next_taken_index];
			this_bb->total_sample_count += loop_asm->total_sample_count;
			if(this_bb->total_sample_count > max_bb_count)max_bb_count = this_bb->total_sample_count;
//		these are defined by the last instruction of the block
			this_bb->end_address = loop_asm->address;
			this_bb->call = loop_asm->call;
			this_bb->branch = loop_asm->branch;
			this_bb->encoding = loop_asm->encoding;
#ifdef DBUG
			fprintf(stderr," this_bb address = 0x%"PRIx64", end_address = 0x%"PRIx64"\n",this_bb->address, this_bb->end_address);
#endif

			if(max_file_line > 0)
				{
				if(loop_asm->principal_source_line < min_file_line)fprintf(stderr," principal_source_line = %d LT min_file_line = %d\n",loop_asm->principal_source_line,min_file_line);
				if(loop_asm->principal_source_line > max_file_line)fprintf(stderr," principal_source_line = %d GT max_file_line = %d\n",loop_asm->principal_source_line,max_file_line);
				source_sample_count[loop_asm->principal_source_line-min_file_line]+=loop_asm->total_sample_count;
				if(source_sample_count[loop_asm->principal_source_line-min_file_line] > sample_count_max)
					{
					sample_count_max = source_sample_count[loop_asm->principal_source_line-min_file_line];
					this_bb->source_line = loop_asm->principal_source_line;
					}
				}
//		use next_taken_stack to count the number of taken branch targets that can get you to the current instruction
//			this is the basic block execution count, 
//			which equals the instruction_retired count for each instruction in the BB
//			undetected targets of indirect branches could cause the instruction count to change within the BB
//			test to update the stack
			if(next_taken_stack != NULL)
				{
				next_taken_stack_loop = next_taken_stack;
				if(next_taken_stack->address < loop_asm->address)
					{
//					pop off the stack, decrement count, free next_taken_stack_loop
#ifdef DBUG
					fprintf(stderr,"decrementing taken stack count from %lu by %lu at 0x%"PRIx64"\n",
						next_taken_count_sum, next_taken_stack_loop->count, loop_asm->address);
#endif
					next_taken_count_sum -= next_taken_stack_loop->count;
					next_taken_stack = next_taken_stack_loop->next;
					if(next_taken_stack != NULL)next_taken_stack->previous = NULL;
					free(next_taken_stack_loop);
#ifdef DBUG
					fprintf(stderr,"after decrement, next_taken_count_sum = %lu at address 0x%"PRIx64"\n",
						next_taken_count_sum, loop_asm->address);
#endif
					}
				}
//		test if current address is the target of branches and add counts to stack and increment stack
			valid_branch_target = 0;
			if(loop_asm->next_taken_list != NULL)
				{
				next_taken_loop = loop_asm->next_taken_list;
				next_taken_stack_loop = next_taken_stack;
				previous_stack = NULL;
// 	outer loop is over next source addresses in the linked list for the current asm line, 
//			current asm line should be a branch target for the list to be != NULL
				while(next_taken_loop != NULL)
					{
					next_taken_address = next_taken_loop->address;
#ifdef DBUG
		fprintf(stderr," instruction IP = 0x%"PRIx64", next_taken_address = 0x%"PRIx64", count = %d\n",
				loop_asm->address, next_taken_address,next_taken_loop->count);
#endif
//				test for valid next_taken_branch address
					if(next_taken_address > end)
						{
#ifdef DBUG
						fprintf(stderr," function %s has taken branch LBR source outside of function, LBR address = 0x%"PRIx64", current address = 0x%"PRIx64"\n",
							this_function->function_name,next_taken_address,loop_asm->address);
#endif
						next_taken_loop = next_taken_loop->next;
						continue;
						}
					else if(next_taken_address < loop_asm->address)
						{
#ifdef DBUG
						fprintf(stderr," function %s has taken branch LBR source previous to current address, LBR address = 0x%"PRIx64", current address = 0x%"PRIx64"\n",
							this_function->function_name,next_taken_address,loop_asm->address);
#endif
						next_taken_loop = next_taken_loop->next;
						continue;
						}
//			valid address for next taken branch instruction, therefore find correct position in next_taken_stack
					else
						{
						valid_branch_target = 1;
#ifdef DBUG
	fprintf(stderr," valid lbr address = 0x%"PRIx64"\n",next_taken_address);
#endif
						if(next_taken_stack == NULL)
							{
//					create new stack as there no longer is one, initialize next_taken_count_sum
							next_taken_stack = (next_taken_struc_ptr) calloc(1,sizeof(next_taken_data));
							if(next_taken_stack == NULL)
								err(1,"failed to malloc next_taken_data stack in func_asm");
							next_taken_stack->address = next_taken_loop->address;
							next_taken_stack->count = next_taken_loop->count;
							next_taken_count_sum = next_taken_loop->count;
#ifdef DBUG
						fprintf(stderr,"created next_taken_struc as stack was NULL for address 0x%"PRIx64", count = %lu\n",
								next_taken_stack->address,next_taken_stack->count);
#endif
							}
						else
							{
//					add the new address to the active stack but put it into the stack in increasing address order
#ifdef DBUG
	fprintf(stderr," valid lbr address, stack not NULL = 0x%"PRIx64"\n",(uint64_t)next_taken_stack);
#endif
							next_taken_stack_loop = next_taken_stack;
							previous_stack = NULL;
							while(next_taken_stack_loop->address < next_taken_address)
								{
#ifdef DBUG
	fprintf(stderr," valid lbr address = 0x%"PRIx64", stack address = 0x%"PRIx64", stack not NULL= 0x%"PRIx64", walking stack\n",
			next_taken_address, next_taken_stack_loop->address,(uint64_t)next_taken_stack_loop);
#endif
								previous_stack = next_taken_stack_loop;
								next_taken_stack_loop = next_taken_stack_loop->next;
								if(next_taken_stack_loop == NULL)break;
								}
							if(next_taken_stack_loop == NULL)
								{
#ifdef DBUG
	fprintf(stderr," valid lbr address, stack not NULL, but next_taken_stack_loop is\n");
#endif
								this_next_taken = (next_taken_struc_ptr) calloc(1,sizeof(next_taken_data));
								if(this_next_taken == NULL)
									err(1,"failed to malloc next_taken_data in func_asm");
								this_next_taken->next = next_taken_stack_loop;
								this_next_taken->address = next_taken_address;
								this_next_taken->count = next_taken_loop->count;
								next_taken_count_sum += next_taken_loop->count;
								if(previous_stack != NULL)
									{
									this_next_taken->previous = previous_stack;
									previous_stack->next = this_next_taken;
									}
#ifdef DBUG
			fprintf(stderr,"created next_taken_struc to insert new entry at end for address 0x%"PRIx64", count = %lu, sum = %lu\n",
								next_taken_address,next_taken_stack->count,next_taken_count_sum);
#endif
								}
//				found existing struc in stack, increment the count for the stack element with the current next_taken_address
							else if(next_taken_stack_loop->address > next_taken_address)
								{
#ifdef DBUG
	fprintf(stderr," valid lbr address, stack not NULL, but next_taken_stack_loop address 0x%"PRIx64" > next_taken_address 0x%"PRIx64"\n",
			next_taken_stack_loop->address, next_taken_address);
#endif
								this_next_taken = (next_taken_struc_ptr) calloc(1,sizeof(next_taken_data));
								if(this_next_taken == NULL)
									err(1,"failed to malloc next_taken_data in func_asm");
								this_next_taken->next = next_taken_stack_loop;
								this_next_taken->address = next_taken_address;
								this_next_taken->count = next_taken_loop->count;
								next_taken_count_sum += next_taken_loop->count;
#ifdef DBUG
				fprintf(stderr,"created next_taken_struc to insert new entry for address 0x%"PRIx64", count = %lu, sum = %lu\n",
								next_taken_address,next_taken_stack->count,next_taken_count_sum);
#endif
								if(previous_stack != NULL)
									{
#ifdef DBUG
				fprintf(stderr," new address, 0x%"PRIx64" should be between 0x%"PRIx64" and 0x%"PRIx64"\n",
						next_taken_address, previous_stack->address, next_taken_stack_loop->address);
#endif
									this_next_taken->previous = previous_stack;
									previous_stack->next = this_next_taken;
									next_taken_stack_loop->previous = this_next_taken;
									}
								else
									{
//							inserting at top of stack
#ifdef DBUG
				fprintf(stderr," new address, 0x%"PRIx64" should be before 0x%"PRIx64"\n",
						next_taken_address, next_taken_stack_loop->address);
#endif
									this_next_taken->previous = NULL;
									next_taken_stack_loop->previous = this_next_taken;
									next_taken_stack = this_next_taken;
									}
								}
//				found existing struc in stack, increment the count for the stack element with the current next_taken_address
							else
								{
#ifdef DBUG
	fprintf(stderr," valid lbr address, stack not NULL, next_taken_stack_loop address 0x%"PRIx64" = next_taken_address 0x%"PRIx64"\n",
			next_taken_stack_loop->address, next_taken_address);
#endif
								next_taken_stack_loop->count += next_taken_loop->count;
								next_taken_count_sum += next_taken_loop->count;
#ifdef DBUG
				fprintf(stderr,"found next_taken_struc entry for address 0x%"PRIx64", count = %lu, sum = %lu\n",
								next_taken_stack_loop->address,next_taken_stack_loop->count,next_taken_count_sum);
#endif
								}
							}
						}
					next_taken_loop = next_taken_loop->next;
					}
				}				
//			test for active stack and update BB struc, asm struc, func_struc, module_struc and principal_process_struc 
			if((next_taken_stack != NULL) && (loop_asm->asm_text != NULL) && (bb_exec_index != 0))
				{
#ifdef DBUG
				fprintf(stderr,"before sum, next_taken_count_sum = %lu at address 0x%"PRIx64"\n",
					next_taken_count_sum, loop_asm->address);
#endif
				this_bb->sample_count[bb_exec_index] = next_taken_count_sum;
				this_bb->sample_count[sw_inst_retired_index] += next_taken_count_sum;
				loop_asm->sample_count[sw_inst_retired_index] = next_taken_count_sum;
				this_function->sample_count[sw_inst_retired_index] += next_taken_count_sum;
#ifdef DBUG
				if(valid_branch_target == 1)
					{
					fprintf(stderr," valid branch target finished total count = %lu stack follows\n",next_taken_count_sum);
					next_taken_stack_loop = next_taken_stack;
					while(next_taken_stack_loop != NULL)
						{
						fprintf(stderr," address = 0x%"PRIx64", count = %lu\n",
							next_taken_stack_loop->address,next_taken_stack_loop->count);
						next_taken_stack_loop = next_taken_stack_loop->next;
						}
					}
#endif
				}
			old_loop_asm = loop_asm;
			loop_asm = loop_asm->next;
			if(loop_asm == NULL)break;
			}
//		check if last asm is a conditional branch
//		if((old_loop_asm->asm_text[1] == 'j' ) && (old_loop_asm->asm_text[2] != 'm'))
		if(loop_asm == NULL)loop_asm = old_loop_asm;
		if(old_loop_asm->conditional == 1)
			{
//	bb terminated by conditional branch & has 2 targets
#ifdef DBUG
			fprintf(stderr," Basic Block %d <0x%"PRIx64"><0x%"PRIx64">\0",bb_count+1,old_loop_asm->target,loop_asm->address);
#endif
			sprintf(this_bb->text," Basic Block %d <0x%"PRIx64"><0x%"PRIx64">\0",bb_count+1,old_loop_asm->target,loop_asm->address);
#ifdef DBUG
			fprintf(stderr," BB %d text = %s\n",k,this_bb->text);
#endif
			this_bb->target1 = old_loop_asm->target;
			this_bb->target2 = loop_asm->address;
#ifdef DBUG
			fprintf(stderr," 2 targets set for this block, known conditional\n");
#endif
			}
		else if (old_loop_asm->target != 0)
			{
			sprintf(this_bb->text," Basic Block %d <0x%"PRIx64">\0",bb_count+1,old_loop_asm->target);
			this_bb->target1 = old_loop_asm->target;
#ifdef DBUG
			fprintf(stderr," 1 target set for this block, known unconditional target = 0x%"PRIx64", branch = %d, addr = 0x%"PRIx64"\n",
				old_loop_asm->target,old_loop_asm->branch, this_bb->end_address);
			fprintf(stderr," old_loop_asm_address = 0x%"PRIx64"\n",old_loop_asm->address);
#endif
			}
		else
			{
			if(loop_asm != NULL)
				{
				sprintf(this_bb->text," Basic Block %d <0x%"PRIx64">\0",bb_count+1,loop_asm->address);
				this_bb->target2 = loop_asm->address;
#ifdef DBUG
			fprintf(stderr," unknown target for this block, target set to next instruction\n");
#endif
				}
			else
				{
				sprintf(this_bb->text," Basic Block %d \0",bb_count+1);
				this_bb->target2 = 0;
#ifdef DBUG
			fprintf(stderr," unknown target for last block, target set to 0\n");
#endif
				}
			}
		if((this_bb->target1 != 0) && ((this_bb->target1 < base) || (this_bb->target1 > end)))deadbeef++;		
//		last bb
		if(k == last_bb)
			{
			while(loop_asm->address <= end)
				{
				for(tmp2=0; tmp2 < num_events; tmp2++)
					this_bb->sample_count[num_events*(num_cores + num_sockets) + tmp2] +=
						loop_asm->sample_count[num_events*(num_cores + num_sockets) + tmp2];
				if(source_index != 0)
					this_bb->sample_count[source_index] += loop_asm->sample_count[source_index];
				if(target_index != 0)
					this_bb->sample_count[target_index] += loop_asm->sample_count[target_index];
				if(next_taken_index != 0)
					this_bb->sample_count[next_taken_index] += loop_asm->sample_count[next_taken_index];
				this_bb->total_sample_count += loop_asm->total_sample_count;
				if(this_bb->total_sample_count > max_bb_count)max_bb_count = this_bb->total_sample_count;
				loop_asm = loop_asm->next;
				}
			this_bb->end_address = end;
#ifdef DBUG
			fprintf(stderr," this_bb address = 0x%"PRIx64", end_address = 0x%"PRIx64"\n",this_bb->address, this_bb->end_address);
#endif
			}
		old_address = branch_address[k];
		previous_bb = this_bb;
		k = tmp;
		bb_count++;
		}
//	fixup code for functions with a single bb
		if(this_function->first_bb == NULL)
			{
#ifdef DBUG
			fprintf(stderr," entering single BB function fixup code\n");
#endif
			bb_count = 1;
			max_bb_count = 0;
			this_bb = basic_block_struc_create();
			if(this_bb == NULL)
				{
				fprintf(stderr,"failed to create BB struc for function %s, address 0x%"PRIx64", entry %d\n",this_function->function_name,branch_address[k], k);
				err(1,"failed to create BB struc");
				}
			this_function->first_bb = this_bb;
			this_bb->next = NULL;
			this_bb->previous = NULL;
			this_bb->address = this_function->first_asm->address;
			this_bb->end_address = end;
			this_bb->source_line = 0;
	                this_bb->text = (char*)malloc(bb_text_len);
        	        if(this_bb->text == NULL)
                	        {
                        	fprintf(stderr,"failed to create BB struc text for function %s, address 0x%"PRIx64", entry %d\n",this_function->function_name,branch_address[k], k);
	                        err(1,"failed to create BB struc text");
        	                }
				sprintf(this_bb->text," Basic Block %d <0x%"PRIx64"><0x%"PRIx64">\0",bb_count,this_bb->address,end);

			last_bb_struc = this_bb;
			loop_asm = this_function->first_asm;
#ifdef DBUG
			if(loop_asm == NULL)fprintf(stderr," loop_asm = NULL\n");
			fprintf(stderr," single BB function fixup code, entering event sum loop, first = 0x%"PRIx64", end = 0x%"PRIx64"\n", loop_asm->address, end);
#endif
                        while((loop_asm != NULL) && (loop_asm->address <= end))
                                {
                                for(tmp2=0; tmp2 < num_events; tmp2++)
                                        this_bb->sample_count[num_events*(num_cores + num_sockets) + tmp2] +=
                                                loop_asm->sample_count[num_events*(num_cores + num_sockets) + tmp2];
                                if(source_index != 0)
                                        this_bb->sample_count[source_index] += loop_asm->sample_count[source_index];
                                if(target_index != 0)
                                        this_bb->sample_count[target_index] += loop_asm->sample_count[target_index];
                                if(next_taken_index != 0)
                                        this_bb->sample_count[next_taken_index] += loop_asm->sample_count[next_taken_index];
                                this_bb->total_sample_count += loop_asm->total_sample_count;
                                if(this_bb->total_sample_count > max_bb_count)max_bb_count = this_bb->total_sample_count;
                                loop_asm = loop_asm->next;
                                }
#ifdef DBUG
			fprintf(stderr," single BB function fixup code, finished event sum loop\n");
#endif
			}
//	end of loop over BB's


//	print out the nodes, with the BB's first
	this_bb = this_function->first_bb;
	while(this_bb != NULL)
		{
		color_index = 9.*(float)this_bb->total_sample_count/(float)max_bb_count + 1;
		fillcolor = (int)color_index;
		if(fillcolor < 1) fillcolor = 1;
		if(fillcolor > 9) fillcolor = 9;
		fprintf(dot,"\t\"Basic Block %d\" [fillcolor=%d];\n",this_bb->block_count,fillcolor);
		this_bb = this_bb->next;
		}
//	for(k=0; k<bb_count; k++)fprintf(dot,"\t\"Basic Block %d\";\n",k+1);
	for(k=0; k<deadbeef; k++)fprintf(dot,"\t\"Addr %d\";\n",k+1);
	fprintf(dot,"\n");

//	create the bb list array

	this_bb = this_function->first_bb;
	bb_addr_list = (addr_list_data*)malloc((bb_count + 1)*sizeof(addr_list_data));
	if(bb_addr_list == NULL)
		{
		fprintf(stderr," failed to create bb addr list array for function %s\n",this_function->function_name);
		err(1, "failed to create addr list array\n");
		}
#ifdef DBUG
	fprintf(stderr," the bb_count = %d here they are\n",bb_count);
#endif
//
	k = 0;
	while(this_bb != NULL)
		{
		if(k >= bb_count)break;
		bb_addr_list[k].base = this_bb->address;
		bb_addr_list[k].len = this_bb->end_address - this_bb->address;
		bb_addr_list[k].struc_ptr = (void*)this_bb;
		last_bb_end = this_bb->end_address;
		if(bb_exec_index != 0)
			this_function->sample_count[bb_exec_index] += this_bb->sample_count[bb_exec_index];
#ifdef DBUG
		fprintf(stderr," this_bb %d, address = 0x%"PRIx64", end_address = 0x%"PRIx64", len = 0x%"PRIx64"",k,this_bb->address, this_bb->end_address,bb_addr_list[k].len);
		if(bb_exec_index != 0)
			{
			fprintf(stderr,", bb_exec = %d\n",this_bb->sample_count[bb_exec_index]);
			}
		else
			{
			fprintf(stderr,"\n");
			}
#endif
		this_bb = this_bb->next;
#ifdef DBUG
		if(this_bb == NULL)fprintf(stderr,"this_bb = NULL, k = %d, bb_count = %d, base = 0x%"PRIx64", end =  = 0x%"PRIx64", first_asm = 0x%"PRIx64", last_asm = 0x%"PRIx64"\n",k,bb_count,base,end,first_asm,last_asm);
#endif
		k++;
		}
#ifdef DBUG
	fprintf(stderr,"finished loop over BB's, bb_exec_index = %d, sw_inst_retired_index = %d\n",
		bb_exec_index,sw_inst_retired_index);
#endif
	if(bb_exec_index != 0)
		{
		this_module->sample_count[bb_exec_index] += this_function->sample_count[bb_exec_index];
		this_module->sample_count[sw_inst_retired_index] += this_function->sample_count[sw_inst_retired_index];
		this_process->sample_count[bb_exec_index] += this_function->sample_count[bb_exec_index];
		this_process->sample_count[sw_inst_retired_index] += this_function->sample_count[sw_inst_retired_index];
		global_sample_count[bb_exec_index] += this_function->sample_count[bb_exec_index];
		global_sample_count[sw_inst_retired_index] += this_function->sample_count[sw_inst_retired_index];
		}

//	do the binary search on the targets to connect the BB's by number
//		then printout the contents to the dot file
#ifdef DBUG
	fprintf(stderr,"finished constructing bb_addr_list with %d elements, lasst_bb_struc->end_address = 0x%"PRIx64"\n",bb_count,last_bb_struc->end_address);
#endif

	this_bb = this_function->first_bb;
	deadbeef = 0;
	j = 0;
	while(this_bb != NULL)
		{
		j++;
		if(this_bb->target1 != 0)
			{
			if((this_bb->target1 >= base) && (this_bb->target1 <= last_bb_end))
				{
//		target1 is inside the function
#ifdef DBUG
				fprintf(stderr,"calling binsearch for address 0x%"PRIx64"\n",this_bb->target1);
#endif
				target_bb = (basic_block_struc_ptr) binsearch(bb_addr_list,bb_count,this_bb->target1);
#ifdef DBUG
				fprintf(stderr," returned from binsearch, target_bb = 0x%p\n", target_bb);
				fprintf(stderr,"\t\"Basic Block %d\"->\"Basic Block %d\";\n",j,target_bb->block_count);
#endif
				fprintf(dot,"\t\"Basic Block %d\"->\"Basic Block %d\";\n",j,target_bb->block_count);
				}
			else
				{
				deadbeef++;
#ifdef DBUG
				fprintf(stderr,"\t\"Basic Block %d\"->\"Addr %d\";\n",j,deadbeef);
#endif
				fprintf(dot,"\t\"Basic Block %d\"->\"Addr %d\";\n",j,deadbeef);
				if(this_bb->call != 0)
					fprintf(dot,"\t\"Addr %d\"->\"Basic Block %d\";\n",deadbeef,this_bb->block_count+1);
				}
			}
		if(this_bb->target2 != 0)
			{
#ifdef DBUG
			fprintf(stderr,"\t\"Basic Block %d\"->\"Basic Block %d\";\n",j,this_bb->block_count+1);
#endif
			fprintf(dot,"\t\"Basic Block %d\"->\"Basic Block %d\";\n",j,this_bb->block_count+1);
			}
		this_bb = this_bb->next;
		}
	fprintf(dot,"}\n");
	fclose(dot);
		
//	create svg command string
	svg_cmd = (char*)malloc(20 + 2*cfg_len + 2*cfg_name_len);
	sprintf(svg_cmd,"dot -Tsvg %s%d%s > %s%d%s\0",cfg,hotspot_index,cfg_name,cfg,hotspot_index,svg_name);
#ifdef DBUG
	fprintf(stderr," %d svg command = %s\n",i,svg_cmd);
#endif
	if(bb_count < max_bb)
		{
		ret_val = system(svg_cmd);
		if(ret_val == -1)fprintf(stderr,"system call of dot -Tsvg failed in func_asm");
		}
//	deal with single bb case

//	print out the asm spreadsheet
#ifdef DBUG
	fprintf(stderr,"finished making bb strucs, starting spreadsheet printing\n");
#endif

//	start printing asm spreadsheet top rows
	fprintf(list,"[\n");
	fprintf(list,"[,\"bb\",\"Address\",\"Princ_L#\",\"Principal File\",\"Init_L#\",\"Initial File\",\"Disassembly\",");
	for(k=0; k < num_col; k++)fprintf(list," \"%s\",",global_event_order->order[k].name);
	fprintf(list," ],\n");
	fprintf(list,"[,");
	for(k=0;k<2;k++)fprintf(list,"\"%d:0\",",k);
	fprintf(list,"\"2:3\",\"2_1:0\",\"2_2:0\",\"2_3:0\",\"3:0\",");
	for(k=0;k<num_col;k++)fprintf(list,"\"%d%s\",",4+global_event_order->order[k].base_col,global_event_order->order[k].ctrl_string);
	fprintf(list," ],\n");
	fprintf(list,"[,,,,,,,\"MSR Programmings\",");
	for(k=0; k < num_col; k++)fprintf(list," 0x%"PRIx64",",global_event_order->order[k].config);
	fprintf(list," ],\n");
	fprintf(list,"[,,,,,,,\"Periods\",");
	for(k=0; k < num_col; k++)fprintf(list," %ld,",global_event_order->order[k].Period);
	fprintf(list," ],\n");
	fprintf(list,"[,,,,,,,\"Multiplex\",");
	for(i=0; i < num_col; i++)fprintf(list," %5.4lf,",global_event_order->order[i].multiplex);
	fprintf(list," ],\n");
	fprintf(list,"[,,,,,,,\"Penalty\",");
	for(k=0; k < num_col; k++)fprintf(list," %d,",global_event_order->order[k].penalty);
	fprintf(list," ],\n");
	fprintf(list,"[,,,,,,,\"Cycles\",");
	for(k=0; k < num_col; k++)fprintf(list," %d,",global_event_order->order[k].cycle);
	fprintf(list," ],\n");

	this_bb = this_function->first_bb;
	loop_asm = this_function->first_asm;
#ifdef DBUG
	if(this_bb == NULL)fprintf(stderr," this_bb = NULL\n");
	if(loop_asm == NULL)fprintf(stderr," loop_asm = NULL\n");
	fprintf(stderr," first_bb address = 0x%"PRIx64", loop_asm address = 0x%"PRIx64"\n",this_bb->address, loop_asm->address);
#endif
	k = 0;
	tmp2=0;
	while((k < bb_count) && (loop_asm != NULL) && (this_bb != NULL))
		{
#ifdef DBUG
		fprintf(stderr,"%d, pointers address, src_line, bb text %p, %p, %p",k+1,&(this_bb->address), &(this_bb->source_line), &(this_bb->text));
		fprintf(stderr," address 0x%"PRIx64"\n",this_bb->address);
		fprintf(stderr," line %d\n",this_bb->source_line);
		text_len = strlen(this_bb->text);
		fprintf(stderr,"text_len = %d\n",text_len);
		fprintf(stderr," text %s\n",this_bb->text);
		fprintf(stderr,"[,%d,0x%"PRIx64",%d,,,, \"%s\",",k+1,this_bb->address, this_bb->source_line, this_bb->text);
		for(j=0; j<num_col; j++)fprintf(stderr," %d,",this_bb->sample_count[ global_event_order->order[j].index]);
#endif

		fprintf(list,"[,%d,\"0x%"PRIx64"\",%d,,,, \"%s\",",k+1,this_bb->address, this_bb->source_line, this_bb->text);
		branch_eval(this_bb->sample_count);
		for(j=0; j<num_col; j++)fprintf(list," %d,",this_bb->sample_count[ global_event_order->order[j].index]);
		fprintf(list," ],\n");
#ifdef DBUG
		fprintf(stderr," this_bb address = 0x%"PRIx64", loop_asm address = 0x%"PRIx64", k = %d, bb end address = 0x%"PRIx64"\n",
					this_bb->address, loop_asm->address, k,this_bb->end_address);
#endif
		while ((loop_asm->address <= this_bb->end_address))
			{
			tmp2++;
#ifdef DBUG
//      follow a single address through if there are worries about lost samples/rva struc's
//		if(loop_asm->address == 0x43e110)fprintf(stderr,"func_asm2 for address 0x43e110, cycle count = %d\n",
//				loop_asm->sample_count[num_events*(num_cores + num_sockets)]);
//		if(loop_asm->address == 0x43e111)fprintf(stderr,"func_asm2 for address 0x43e111, cycle count = %d\n",
//				loop_asm->sample_count[num_events*(num_cores + num_sockets)]);
#endif
#ifdef DBUG
			fprintf(stderr,"%d, 0x%"PRIx64",null,\" %s\" ",k+1,loop_asm->address, loop_asm->asm_text);
			for(j=0; j<num_events; j++)fprintf(stderr," %d,",loop_asm->sample_count[num_events*(num_cores + num_sockets) + j]);
			fprintf(stderr,"null],\n");
#endif
//    start at 1 to get rid of the tab
//	do not printout the intial source info if it is the same as the final source info
			if((loop_asm->initial_source_name != NULL) && (loop_asm->principal_source_name != NULL))
				{
				if((strcmp(loop_asm->initial_source_name,loop_asm->principal_source_name) == 0) &&
					(loop_asm->principal_source_line == loop_asm->initial_source_line))
					{
					fprintf(list,"[,%d,\"0x%"PRIx64"\",%d,\"%s\",,\"\",\"%s\", ",k+1,
						loop_asm->address, loop_asm->principal_source_line,
						loop_asm->principal_source_name,
						&loop_asm->asm_text[1]);
					}
				else
					{
					fprintf(list,"[,%d,\"0x%"PRIx64"\",%d,\"%s\",%d,\"%s\",\"%s\", ",k+1,
						loop_asm->address, loop_asm->principal_source_line,
						loop_asm->principal_source_name, loop_asm->initial_source_line, loop_asm->initial_source_name,
						&loop_asm->asm_text[1]);
					}
				}
			else
				{
				fprintf(list,"[,%d,\"0x%"PRIx64"\",%d,\"%s\",%d,\"%s\",\"%s\", ",k+1,
					loop_asm->address, loop_asm->principal_source_line,
					loop_asm->principal_source_name, loop_asm->initial_source_line, loop_asm->initial_source_name,
					&loop_asm->asm_text[1]);
				}
			branch_eval(loop_asm->sample_count);
			for(j=0; j<num_col; j++)fprintf(list," %d,",loop_asm->sample_count[ global_event_order->order[j].index ]);
			fprintf(list," ],\n");
			loop_asm = loop_asm->next;
			if(loop_asm == NULL)break;
			}
#ifdef DBUG
		fprintf(stderr," increment function %s bb_exec count by %d, to %d, bb_exec_index = %d\n",this_function->function_name,
			this_bb->sample_count[bb_exec_index], this_function->sample_count[bb_exec_index], bb_exec_index);
#endif
		this_bb = this_bb->next;
		k++;
		}

//	fprintf(stderr," module sw inst = %d, bb_exec = %d\n",
//		this_module->sample_count[sw_inst_retired_index],this_module->sample_count[bb_exec_index]);
//	fprintf(stderr," process sw inst = %d, bb_exec = %d\n",
//		this_process->sample_count[sw_inst_retired_index],this_process->sample_count[bb_exec_index]);
//	fprintf(stderr," global sw inst = %d, bb_exec = %d\n",
//		global_sample_count[sw_inst_retired_index],global_sample_count[bb_exec_index]);

#ifdef DBUG
	fprintf(stderr,"finished loop of printing asm/BB's\n");
#endif
//		final data row is the function total
	branch_eval(this_function->sample_count);
	this_function->called_branch_eval = 1;
	fprintf(list,"[,%d,,,,,, \"%s\",",k+1,this_function->function_name);
//	branch_eval already called from hotlist_function
	for(j=0; j<num_col; j++)fprintf(list," %d,",this_function->sample_count[ global_event_order->order[j].index ]);
	fprintf(list," ],\n");
	fprintf(list,"]\n");

//  insert */ here
	pclose(objout);
	fclose(list);
	return this_function->total_sample_count;
}

void * 
func_src(pointer_data * global_func_list, int index)
{
//	create spreadsheet for source file
	int i,j,k,l,m,n,tmp, tmp2, line_count, asm_count,hotspot_index;
	FILE * list, *src_file;
	int access_status;
	char spread[]="./spreadsheets", asmd[]="./spreadsheets/asm/", cfg[]="./spreadsheets/cfg/", src[]="./spreadsheets/src/";
	char  sheetname[] = "_src.csv", obj1[] = "objdump -d --start-address=0x", obj2[] = " --stop-address=0x";
	char* spreadsheet;
	char* local_path;
	char sources[]="./sources/";
	char null_string[] = " null";
	int null_string_len=5, filename_len, src_len, src_path_len,src_dir_len;
	char * funcname, *filename, *obj_cmd, line_buf[1024];
	char mode[] = "w+";
	char field1[1024], field2[1024],field3[1024],target_address[80],byte_field[3];
	int base_char;
	size_t funcname_len, spreadsheet_len = 20, line_buf_len = 1024, buf_len, obj1_len = 29, obj2_len = 18, module_len, asm_len;
	function_struc_ptr this_function;
	module_struc_ptr this_module;
	sample_struc_ptr loop_rva;
	asm_struc_ptr this_asm=NULL, next_asm=NULL, previous_asm=NULL, loop_asm=NULL, old_loop_asm;
	basic_block_struc_ptr this_bb=NULL, next_bb=NULL, previous_bb=NULL, loop_bb=NULL;
	int *sample_count;
	float summed_samples, total_samples;
	size_t base, end;
	int count, branch, branch_count, first_bb, last_bb, bb_count;
	uint64_t address, old_address, end_address, *branch_address,byte_val;
	const char * source_file, *source_file_old;
	int kk,src_file_path_len, ret_val;
	unsigned int line_nr, line_nr_old;
	file_list_struc_ptr principal_file_loop,old_principal_file,principal_file_max=NULL;
	int max_principal_count = -1, num_col;
	int first_line_number = 100000000, last_line_number = 0, num_src_lines, max_src_lines, initial_line = -1, previous_line = -1;
	source_line_struc_ptr*  source_line_ptr_array, this_source_line, previous_source_line, loop_source_line;

	spreadsheet_len = strlen(sheetname);
	src_len = strlen(src);
	src_dir_len = strlen(sources);
	total_samples = global_sample_count_in_func;
	summed_samples = 0;
	i = index;
	hotspot_index = global_func_count - 1 - index;

	branch_count = 0;
	this_asm = NULL;
	previous_asm = NULL;
	this_bb = NULL;
	previous_bb = NULL;
	previous_source_line = NULL;
	
	this_function = (function_struc_ptr) global_func_list[i].ptr;
	if(this_function->principal_file == NULL)
		{
//	no symbols were available
		fprintf(stderr,"no principal_file struc for function %s so no source file was made\n",this_function->function_name);
		return;
		}
	if(this_function->principal_file->principal_file_name == NULL)
		{
//	no symbols were available
		fprintf(stderr,"no symbols were available for function %s so no source file was made\n",this_function->function_name);
		return;
		}
	count = global_func_list[i].val;
	this_module = this_function->this_module;
	loop_rva = this_function->first_rva;
#ifdef DBUG
	fprintf(stderr," func_asm index = %d, %d, function %s, first rva = 0x%"PRIx64", base = 0x%"PRIx64", len = %lx\n",i,hotspot_index,this_function->function_name, loop_rva->rva,this_function->function_rva_start, this_function->function_length);
	fprintf(stderr," src_len = %d, src_dir_len = %d\n",src_len,src_dir_len);
#endif
#ifdef DBUG
	fprintf(stderr,"from func_src: this_function->principal file path = %s\n",this_function->principal_file->path);
	fprintf(stderr,"this_function->principal file name = %s\n",this_function->principal_file->principal_file_name);
#endif

	funcname_len = strlen(this_function->function_name);
	filename = (char*) malloc((20 + src_len + spreadsheet_len)*sizeof(char));
	if(filename == NULL)
		{
		fprintf(stderr,"failed to malloc src spreadsheet filename for function %s\n",this_function->principal_file->principal_file_name);
		err(1,"failed to malloc src spreadsheet");
		}
	for(j=0; j < src_len; j++)filename[j]=src[j];
	sprintf(&filename[src_len],"%d%s\0",hotspot_index,sheetname);
	filename_len = strlen(filename);
#ifdef DBUG
	fprintf(stderr," %d len = %d, filename = %s\n",i,filename_len, filename);
#endif
//	find source file
#ifdef DBUG
	fprintf(stderr,"principal file struc = %p\n",this_function->principal_file);
	fprintf(stderr,"file name address = %p\n",this_function->principal_file->principal_file_name);
	fprintf(stderr,"file name = %s\n",this_function->principal_file->principal_file_name);
#endif
	src_path_len = strlen(this_function->principal_file->principal_file_name);
	local_path = (char*)malloc(src_dir_len + src_path_len + 1);
	if(local_path == NULL)
		{
		fprintf(stderr,"failed to malloc space for local src file path, func = %s\n",this_function->function_name);
		err(1, "failed to malloc local src path");
		}
	for(i=0; i<src_dir_len; i++)local_path[i]=sources[i];
	for(i=0; i<src_path_len; i++)local_path[src_dir_len+i] = this_function->principal_file->principal_file_name[i];
	local_path[src_dir_len+src_path_len] = '\0';
#ifdef DBUG
	fprintf(stderr," local_path = %s\n",local_path);
#endif
	access_status = access(local_path, R_OK);
#ifdef DBUG
	fprintf(stderr," local path access = %d, R_OK = %d\n",access_status, R_OK);
#endif

	if(access_status != 0)
		{
		fprintf(stderr," source_file not found in ./sources, try full path,  local_path = %s, at 0x%p, len = %d\n",
			local_path,local_path, src_dir_len + src_path_len);
		free(local_path);
		local_path = this_function->principal_file->path;
#ifdef DBUG
		fprintf(stderr," local_path after not having found the file in sources= %s\n",local_path);
#endif
		if (subst_path_prefix[0] && subst_path_prefix[1])
			{
				char *p = strstr(local_path, subst_path_prefix[0]);
				if (p == local_path)
					{
					char *new_path;
					new_path = malloc(1 + strlen(local_path) - strlen(subst_path_prefix[0]) + strlen(subst_path_prefix[1]));
					if (!new_path)
						err(1, "not enough memory for local_path prefix swap");
					sprintf(new_path, "%s%s", subst_path_prefix[1], local_path + strlen(subst_path_prefix[0]));
#ifdef DBUG
					fprintf(stderr, "new_path=%s\n", new_path);
#endif
//					free(local_path);
					local_path = new_path;
					}
			
			}
		access_status = access(local_path, R_OK);
        fprintf(stderr," local path access after path subst = %d, R_OK = %d\n",access_status, R_OK);

		if(access_status !=0)
			{
			fprintf(stderr,"cannot find source file %s for function %s, module = %s, only asm will be available\n",
				local_path, this_function->function_name,this_module->path);
			return;
			}
		}
//	find minimum and maximum principal source file line numbers
	loop_asm = this_function->first_asm;

#ifdef DBUG
		fprintf(stderr," this function first asm ptr = %p\n",this_function->first_asm);
#endif
	while(loop_asm != NULL)
		{

#ifdef DBUG
		fprintf(stderr," this asm file name = %s, line number = %d\n",loop_asm->principal_source_name,loop_asm->principal_source_line);
#endif
		if(strcmp(loop_asm->principal_source_name, this_function->principal_file->principal_file_name) == 0)
			{
			if(loop_asm->principal_source_line < first_line_number) first_line_number = loop_asm->principal_source_line;
			if(loop_asm->principal_source_line > last_line_number) last_line_number = loop_asm->principal_source_line;
			if(initial_line == -1) initial_line = loop_asm->principal_source_line;
			if(loop_asm->principal_source_line <= 0)
				{
				fprintf(stderr,"source file, %s, for function %s, in module %s, has bad debug information, line number = %d,  <= 0\n",
					this_function->principal_file->principal_file_name,this_function->function_name,this_module->path,
					loop_asm->principal_source_line);
				fprintf(stderr,"                      only asm will be available\n");
				return;
				}
			}
		loop_asm = loop_asm->next;
		}
#ifdef DBUG
	fprintf(stderr,"finished while loop over asm\n");
#endif
	if(first_line_number > last_line_number)
		{
		fprintf(stderr,"source line range not found for function %s, file %s\n",this_function->function_name,local_path);
		return;
		}
	first_line_number=first_line_number-20;
	last_line_number= last_line_number+20;
	if(first_line_number < 1)first_line_number=1;
	num_src_lines = last_line_number - first_line_number + 1;
#ifdef DBUG
	fprintf(stderr," minimum line = %d, maximum line = %d, num_src_lines = %d for file %s\n",first_line_number, last_line_number,num_src_lines,this_function->principal_file->principal_file_name);
#endif

//	allocate an array for pointers to the source line strucs
	source_line_ptr_array = (source_line_struc_ptr*) malloc(num_src_lines* sizeof(source_line_struc_ptr));
	if(source_line_ptr_array == NULL)
		{
		fprintf(stderr,"failed to malloc space for line ptr array for file %s\n",this_function->principal_file->principal_file_name);
		err(1,"failed to malloc line array");
		}

//	open source file and loop through source lines
//	for each line allocate array of structures for line number and line text and sample counts/source line
	src_file = fopen(local_path,"r");
	if(src_file == NULL)
		{
		fprintf(stderr,"failed to open src file = %s\n",local_path);
		err(1,"failed to open src file which had a valid response to access");
		}
	i = 0;
	j = 0;

	while(fgets(line_buf,line_buf_len,src_file) != NULL)
		{
		j++;
		if(j <= first_line_number)continue;
		if(j > last_line_number)break;
#ifdef DBUG
		fprintf(stderr," line %d, text = %s\n",j,line_buf);
#endif
		buf_len = strlen(line_buf);
		this_source_line = source_line_create();
		if(this_source_line == NULL)
			{
			fprintf(stderr,"failed to create source line structure for line %d, file %s\n",i,local_path);
			err(1,"failed to create source structure");
			}
		if(this_function->first_source_line == NULL)this_function->first_source_line = this_source_line;
		this_source_line->previous = previous_source_line;
		if(previous_source_line != NULL)previous_source_line->next = this_source_line;
		this_source_line->line = j;
//	allocate a bunch of extra characters to handle double quotes in the source line text
		this_source_line->source_text = (char*) malloc(buf_len+50);
		if(this_source_line->source_text == NULL)
			{
			fprintf(stderr,"failed to create source test for line %d, file %s\n",i,local_path);
			err(1,"failed to create source text");
			}
		for(k=0; k<buf_len+49; k++)this_source_line->source_text[k] = '\0';
		kk = 0;
		for(k=0; k<buf_len-1; k++)
			{

			if(line_buf[k] == '\\')
				{
				this_source_line->source_text[k+kk] = '\\';
				kk++;
				}
			if(line_buf[k] == '\"')
				{
				this_source_line->source_text[k+kk] = '\\';
				kk++;
				}

			this_source_line->source_text[k+kk] = line_buf[k];
			}
		this_source_line->source_text[k+kk + 1] = '\0';
#ifdef DBUG
		fprintf(stderr," source string = %s\n",this_source_line->source_text);
#endif

		source_line_ptr_array[i] = this_source_line;
		i++;
		previous_source_line = this_source_line;
		}
	max_src_lines = i;
	if(max_src_lines == 0)
		{
//		really bad debug information line number range beyond end of file
		fprintf(stderr,"really bad debug information line number range beyond end of file\n");
		fprintf(stderr," function = %s, first line = %d, last line = %d, source file = %s, len = %d\n",
		this_function->function_name, first_line_number, last_line_number,local_path,j);
		fclose(src_file); 
		return;
		}
//	loop over asm structures and increment source line sample count arrays	
#ifdef DBUG
	fprintf(stderr," finished reading source file max_src_lines = %d, num_src_lines = %d\n",max_src_lines,num_src_lines);
#endif
	loop_asm = this_function->first_asm;
#ifdef DBUG
	fprintf(stderr," first asm struc address = %p\n",loop_asm);
	fprintf(stderr," first asm address = 0x%"PRIx64", %s\n",loop_asm->address,loop_asm->asm_text);
#endif
	i = 0;
	j = 0;
	previous_line = initial_line;
	while(loop_asm != NULL)
		{
#ifdef DBUG
		fprintf(stderr," asm source name = %s, line = %d\n",loop_asm->principal_source_name, loop_asm->principal_source_line);
#endif
			j++;
#ifdef DBUG
		fprintf(stderr,"j = %d, asm = %s\n",j,loop_asm->asm_text);
#endif
		if(strcmp(loop_asm->principal_source_name, this_function->principal_file->principal_file_name) == 0)
			{
//		valid debug data pointing to correct file
			i = loop_asm->principal_source_line - first_line_number - 1;
			if(i < 0)i = 0;
#ifdef DBUG
			fprintf(stderr,"correct principal file, array index = %d\n",i);
#endif
			if(i < max_src_lines)
				{
				this_source_line = source_line_ptr_array[i];
				previous_line = loop_asm->principal_source_line;
				}
			else
				{
				i = previous_line - first_line_number -1;
				if(i < 0) i = 0;
				this_source_line = source_line_ptr_array[i];
				}
			}
		else
			{
			i = previous_line - first_line_number -1;
			if(i < 0)i = 0;
#ifdef DBUG
			fprintf(stderr,"incorrect principal file, array index = %d\n",i);
#endif
			this_source_line = source_line_ptr_array[i];
			}
//	add samples to this source line if the asm struc has samples
//			but only machine totals for now
		if(loop_asm->total_sample_count > 0)
			{
			for(k = 0; k < num_events; k++) 
				this_source_line->sample_count[num_events*(num_cores + num_sockets) + k] += 
					loop_asm->sample_count[num_events*(num_cores + num_sockets) + k];
			if(source_index != 0)
				this_source_line->sample_count[source_index] += loop_asm->sample_count[source_index];			
			if(target_index != 0)
				this_source_line->sample_count[target_index] += loop_asm->sample_count[target_index];			
			if(call_index != 0)
				this_source_line->sample_count[call_index] += loop_asm->sample_count[call_index];
			if(mispredict_index != 0)
				this_source_line->sample_count[mispredict_index] += loop_asm->sample_count[mispredict_index];
			if(indirect_index != 0)
				this_source_line->sample_count[indirect_index] += loop_asm->sample_count[indirect_index];
			}

		loop_asm = loop_asm->next;
		}


//	print out spreadsheet
	num_col = global_event_order->num_fixed + global_event_order->num_ordered;
#ifdef DBUG
	fprintf(stderr,"finished accumulating data from asm to src\n");
#endif
#ifdef DBUG
	fprintf(stderr," spreadsheet filename for file %d = %s\n",index,filename);
#endif
	list = fopen(filename,mode);
	if(list == NULL)
		{
		fprintf(stderr,"failed to open file %s\n",filename);
		err(1,"failed to open asm listing file");
		}
	fprintf(list,"[\n");
	fprintf(list,"[,\"Line Number\",\"Source\",");
	for(k=0; k < num_col; k++)fprintf(list," \"%s\",",global_event_order->order[k].name);
	fprintf(list," ],\n");
	fprintf(list,"[,");
	for(k=0;k<2;k++)fprintf(list,"\"%d:0\",",k);
	for(k=0;k<num_col;k++)fprintf(list,"\"%d%s\",",2+global_event_order->order[k].base_col,global_event_order->order[k].ctrl_string);
	fprintf(list," ],\n");
	fprintf(list,"[,,\"MSR Programming\",");
	for(k=0; k < num_col; k++)fprintf(list," 0x%"PRIx64",",global_event_order->order[k].config);
	fprintf(list," ],\n");
	fprintf(list,"[,,\"Periods\",");
	for(k=0; k < num_col; k++)fprintf(list," %ld,",global_event_order->order[k].Period);
	fprintf(list," ],\n");
	fprintf(list,"[,,\"Multiplex\",");
	for(k=0; k < num_col; k++)fprintf(list," %5.4lf,",global_event_order->order[k].multiplex);
	fprintf(list," ],\n");
	fprintf(list,"[,,\"Penalty\",");
	for(k=0; k < num_col; k++)fprintf(list," %d,",global_event_order->order[k].penalty);
	fprintf(list," ],\n");
	fprintf(list,"[,,\"Cycles\",");
	for(k=0; k < num_col; k++)fprintf(list," %d,",global_event_order->order[k].cycle);
	fprintf(list," ],\n");

//      the data
	for(i=0; i< max_src_lines; i++)
		{
		this_source_line = source_line_ptr_array[i];
		fprintf(list,"[,%d, \"%s\",",this_source_line->line,this_source_line->source_text);
		branch_eval(this_source_line->sample_count);
		for(j=0; j<num_col; j++)fprintf(list," %d,",this_source_line->sample_count[ global_event_order->order[j].index ]);
		fprintf(list," ],\n");
		}


//		final data row is the function total
	fprintf(list,"[,%d, \"%s\",",max_src_lines+1,this_function->function_name);
//	branch_eval already called from hotlist_function
	for(j=0; j<num_col; j++)fprintf(list," %d,",this_function->sample_count[ global_event_order->order[j].index ]);
	fprintf(list," ],\n");
	fprintf(list,"]\n");
	fclose(list);
}

void * 
hot_list(pointer_data * global_func_list)
{
	int i,j,k,tmp, tmp2, line_count, asm_count, index;
	FILE * list, *objout;
	char  spreadsheet[] = "_asm_spreadsheet.csv", obj1[] = "objdump -d --start-address=0x", obj2[] = " --stop-address=0x";
	char * funcname, *filename, *obj_cmd, line_buf[1024];
	char mode[] = "w+";
	char field1[1024], field2[1024],field3[1024],target_address[80];
	size_t funcname_len, spreadsheet_len = 20, line_buf_len = 1024, buf_len, obj1_len = 29, obj2_len = 18, module_len, asm_len;
	function_struc_ptr this_function;
	module_struc_ptr this_module;
	sample_struc_ptr loop_rva;
	asm_struc_ptr this_asm=NULL, next_asm=NULL, previous_asm=NULL, loop_asm=NULL;
	basic_block_struc_ptr this_bb=NULL, next_bb=NULL, previous_bb=NULL, loop_bb=NULL;
	int *sample_count;
	float summed_samples, total_samples;
	size_t base, end;
	int count, branch, branch_count, first_bb, last_bb, bb_count;
	uint64_t address, old_address, end_address, *branch_address;


	total_samples = global_sample_count_in_func;
	summed_samples = 0;

	i = global_func_count - 1;
	j = global_func_count - asm_cutoff;
//	fprintf(stderr,"first index = %d, last index = %d, cuttof = %f,total_samples = %f\n",i, j, sum_cutoff,total_samples);
	while((i >= global_func_count - asm_cutoff) && (summed_samples/total_samples < sum_cutoff))
		{
		this_function = (function_struc_ptr) global_func_list[i].ptr;
#ifdef DBUG
		fprintf(stderr," calling func_asm for element %d, function = %s\n",i,this_function->function_name);
#endif
		summed_samples += (float) func_asm(global_func_list, i);
		func_src( global_func_list, i);
		i--;
#ifdef DBUG
		fprintf(stderr,"i = %d, summed_samples = %f, function_sample_count = %d, %d\n",i,summed_samples,this_function->total_sample_count, count);
#endif
		}
}

void 
create_dir(void)
{
	char spread[]="./spreadsheets", asmd[]="./spreadsheets/asm", cfg[]="./spreadsheets/cfg", src[]="./spreadsheets/src",cg[]="./spreadsheets/cg";
	char mkdir[]="mkdir ", date[]="date", mv_spread[]="mv ./spreadsheets ";
	int spread_len, asm_len, cfg_len, src_len, mkdir_len, exists_len, dir_len, access_status, ret_val;
	char line_buf[2048];
	int line_buf_len = 2048, line_len, date_len;
	char exists[]="File exists";
	char * cmd, *dir[5], *date_str, *new_spread, *mv_cmd;
	int i,j,k;
	FILE * cmdout;
	int exists_test;
	time_t t;
	struct tm tm;
	struct stat st;
	char time_str[32];
	char path[PATH_MAX];
	char command[PATH_MAX];

/*
	spread_len = strlen(spread);
	asm_len = strlen(asmd);
	cfg_len = strlen(cfg);
	src_len = strlen(src);
*/

	if((stat(spread,&st) !=-1) || (errno != ENOENT))
		{
		time(&t);
		localtime_r(&t, &tm);
		strftime(time_str, sizeof(time_str), "%F-%H:%M:%S", &tm);
		sprintf(path,"%s-%s", spread, time_str);
		fprintf(stderr,"move spreadsheets to %s\n",path);
		sprintf(command,"%s%s",mv_spread, path);
		ret_val = system(command);
		if(ret_val == -1)
			err(1,"failed to move spreadsheets in create_dir");
		}
	mkdir_len = strlen(mkdir);
	exists_len = strlen(exists);

	dir[0] = spread;
	dir[1] = asmd;
	dir[2] = cfg;
	dir[3] = src;
	dir[4] = cg;

	for(k = 0; k < 5; k++)
		{
		dir_len = strlen(dir[k]);
		cmd = malloc(mkdir_len + dir_len + 1);
		if(cmd == NULL)
			{
			fprintf(stderr," failed to malloc command buffer in create_dir\n");
			err(1," malloc failed in create_dir");
			}
		for(i=0; i < mkdir_len; i++)cmd[i] = mkdir[i];
		for(i=0; i < dir_len; i++)cmd[mkdir_len + i] = dir[k][i];
		cmd[mkdir_len + dir_len] = '\0';
#ifdef DBUG
		fprintf(stderr," mkdir cmd = %s\n",cmd);
#endif
		cmdout = popen(cmd, "r");
		if(cmdout == NULL)
			{
			fprintf(stderr,"failed to execute popen(mkdir), iter = %d\n",k);
			err(1, "failed mkdir");
			}
		if(fgets(line_buf,line_buf_len,cmdout) != NULL)
			{
//	problem creating directory
#ifdef DBUG
			fprintf(stderr," mkdir output = %s\n",line_buf);
#endif
			line_len = strlen(line_buf);
			if(line_len ==0) continue;
			if(line_len < exists_len)
				{
				fprintf(stderr," failed to create %s in create dir and did not get File Exists, message = %s \n",dir[k],line_buf);
				err(1,"error creating directory");
				}
//	check if string ends with "File exists"
			exists_test = 0;
			for(i=0; i < exists_len; i++)
				if(exists[exists_len - i - 1] != line_buf[line_len - i - 1])exists_test++;
			if(exists_test !=0)
				{
				fprintf(stderr," failed to create %s in create dir and did not get File Exists, message = %s \n",dir[k],line_buf);
				err(1,"error creating directory");
				}
			}
		free(cmd);
		}
	pclose(cmdout);
}

void 
multiplex_correction(void)
{
	int i,j,k;
	double sum1, sum2, sum3;
	
	for(i=0; i<num_events; i++)
		{
		sum1 = 0;
		for(j=0; j<num_cores; j++)
			{
			sum1 += (double)global_sample_count[num_cores*i + j]*global_multiplex_correction[num_cores*i + j];
			if(j == 2)
				{
		fprintf(stderr," event = %d, core = %d, sample count = %d\n",i,j,global_sample_count[num_cores*i + j]);
		fprintf(stderr,"global_multiplex_correction for event %d, core - %d  = %g, sum1 = %g\n",i,j,global_multiplex_correction[num_cores*i + j], sum1);
				}
			}
		global_multiplex_correction[num_events*(num_cores + num_sockets) + i] = 1.0;
		if((sum1 != 0) && (global_sample_count[num_events*(num_cores + num_sockets) + i] != 0))
		global_multiplex_correction[num_events*(num_cores + num_sockets) + i] = sum1/(double)global_sample_count[num_events*(num_cores + num_sockets) + i];
		fprintf(stderr,"global_multiplex_correction for event %d = %5.4f, sum1 = %g\n",i,global_multiplex_correction[num_events*(num_cores + num_sockets) + i], sum1);
		}
//	set the corrections for the empty event and all the branch and sub branch rows
	for(i=0; i < global_event_order->num_branch+global_event_order->num_sub_branch + 1; i++)
		global_multiplex_correction[num_events*(num_cores + num_sockets + 1) + i] = 1.0;
}


void * 
inst_working_set(module_struc_ptr this_module)
{
	sample_struc_ptr loop_rva;
	uint64_t this_cacheline, old_cacheline;
	int num_cachelines,this_cacheline_count, total_count,final_count,interupt_count;
	int i,j,k, event_id = 0;;
	line_data * cachelines;
	FILE *list, *list2;
	char dir[]="./spreadsheets/", file[]="_working_set.txt", file2[]="_sum256.txt";
	char* filename,*filename2;
	char mode[] = "w+";
	int linecount=0, sum256=0;

	filename = (char*)malloc(strlen(dir) + strlen(this_module->module_name) + strlen(file) + 6);
	sprintf(filename,"%s%s%s\0",dir,this_module->module_name,file);
	fprintf(stderr,"from inst_working_set, file = %s\n",filename);
	filename2 = (char*)malloc(strlen(dir) + strlen(this_module->module_name) + strlen(file2) + 6);
	sprintf(filename2,"%s%s%s\0",dir,this_module->module_name,file2);
	fprintf(stderr,"from sum256, file = %s\n",filename2);
	fprintf(stderr,"module = %s, sample_count = %d\n",this_module->path,this_module->total_sample_count);

	cachelines = (line_data *) malloc(this_module->rva_count*sizeof(line_data));
	if(cachelines == NULL)
		{
		fprintf(stderr,"failed to malloc cachelines array in inst_working_set for module %s\n",this_module->path);
		err(1,"malloc fialed in inst_working_set");
		}
//      initialize so the sorter doesn't crash on unused elements
	for(i=0; i<this_module->rva_count;i++)
		{
		cachelines[i].address = 0;
		cachelines[i].sample_count = 0;
		}

	old_cacheline = 0;
	this_cacheline_count = 0;
	num_cachelines = 0;
	total_count = 0;
	final_count = 0;
	interupt_count=0;
	for(i=this_module->rva_count-1; i>=0; i--)
		{
		this_cacheline = (this_module->rva_list[i].val & 0xFFFFFFFFFFFFFFC0);
		if(this_cacheline == old_cacheline)
			{
			loop_rva = this_module->rva_list[i].ptr;
			this_cacheline_count += loop_rva->total_sample_count;
			total_count+=loop_rva->total_sample_count;
			interupt_count+=loop_rva->sample_count[num_events*(num_cores+num_sockets) + event_id];
			cachelines[num_cachelines-1].sample_count = this_cacheline_count;
			}
			else
			{
			num_cachelines++;
			cachelines[num_cachelines-1].address = this_cacheline;
			old_cacheline = this_cacheline;
			loop_rva = this_module->rva_list[i].ptr;
			total_count+=loop_rva->total_sample_count;
			interupt_count+=loop_rva->sample_count[num_events*(num_cores+num_sockets) + event_id];
			this_cacheline_count = loop_rva->total_sample_count;
			cachelines[num_cachelines-1].sample_count = this_cacheline_count;
			}

		}
	fprintf(stderr," total count in rva's = %d, num_cachelines = %d, rva_count = %d\n",total_count,num_cachelines,this_module->rva_count);
	for(i=num_cachelines-1; i>= 0; i--)
		{
		final_count+=cachelines[i].sample_count;
		}
	fprintf(stderr,"final_count = %d, interupt_count = %d\n",final_count,interupt_count);
	final_count = 0;
	quickSort3(cachelines,num_cachelines);
	list = fopen(filename,mode);
	list2 = fopen(filename2,mode);
	if(list == NULL)
		{
		fprintf(stderr,"failed to open file %s\n",filename);
		err(1,"failed to open asm listing file");
		}
	linecount=0;
	sum256=0;
	for(i=num_cachelines-1; i>= 0; i--)
		{
		final_count+=cachelines[i].sample_count;
		sum256+=cachelines[i].sample_count;
		linecount++;
		fprintf(list," 0x%"PRIx64", %d\n",cachelines[i].address,cachelines[i].sample_count);
		if(linecount == 256)
			{
			linecount = 0;
			fprintf(list2," %d\n",sum256);
			sum256=0;
			}
		}
	fprintf(list2," %d\n",sum256);
	fprintf(stderr,"final_count = %d, interupt_count = %d\n",final_count,interupt_count);
	free(filename);
	free(filename2);
	free(cachelines);
	return;
}

