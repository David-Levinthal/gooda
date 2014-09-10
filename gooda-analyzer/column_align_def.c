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



//      fixed
char cycles[]="unhalted_core_cycles",stalls[]="uops_retired:stall_cycles",inst[]="instruction_retired";
char uops[]="uops_retired:any";


#define NUM_FIXED 4
#define NUM_BRANCH 0
#define TOT_SUB_BRANCH 0
#define LARGEST 20

#define NUM_SUB_BRANCH 0
#define CTRL_STRING_LEN 20

char * column_name[NUM_BRANCH+1][LARGEST];
int   column_index[NUM_BRANCH+1][LARGEST];
int   num_elements[NUM_BRANCH+1], branch_index[NUM_BRANCH];
int   num_primary_elements[NUM_BRANCH+1];
int * fixed_index;
char * branch_name[NUM_BRANCH];
char * branch_sub_name[NUM_BRANCH][NUM_SUB_BRANCH];

void	
init_order_def()
{


	event_order_struc_ptr this_event_order;
	int i,j,k,l,len, found_fixed_events, found_ordered_events;
	

	column_name[0][0] = cycles;
	column_name[0][1] = stalls;
	column_name[0][2] = inst;
	column_name[0][3] = uops;
	num_primary_elements[0] = NUM_FIXED;
	num_elements[0] = NUM_FIXED;

#ifdef DBUG
	fprintf(stderr,"fixed assignments finished in init_order\n");
#endif

	for(i=0; i< num_events; i++)event_list[i].fixed = 0;

	this_event_order = (event_order_struc_ptr)malloc(sizeof(event_order_data));
	if(this_event_order == NULL)
		{
		fprintf(stderr,"failed to create event_order_struc\n");
		err(1,"failed to malloc event order struc");
		}

	for(i=0; i< NUM_BRANCH+1; i++)this_event_order->num_fixed += num_elements[i];
	this_event_order->num_ordered = 0;
	fixed_index = (int *)malloc(this_event_order->num_fixed*sizeof(int));
	if(fixed_index == NULL)
		{
		fprintf(stderr,"failed to malloc fixed_index array\n");
		err(1,"failed to malloc fixed_index");
		}

//	initialize the fixed indecies to the zero value column
//      if the event name for a given entry is found to have been collected
//      the value will be reset to the index into the sample data array
//      as all granularity (process, module, function, source line, asm and rva) structures have identically sized sample data arrays
//      this index mapping is good for all tables Gooda generates


	for(i=0; i < this_event_order->num_fixed; i++)fixed_index[i] = num_events*(num_cores + num_sockets + 1);	

#ifdef DBUG
	fprintf(stderr,"init order, about to start triple loop to find fixed events\n");
#endif

//      loop over the branches and the components of each branch and for each branch component 
//      find the matching event name in the perf.data header data structure array, event_list[k]

	l = 0;
	for(i=0; i<NUM_BRANCH+1; i++)
		{
		j = 0;
		while(j < num_elements[i])
			{
#ifdef DBUG
			fprintf(stderr," name [%d][%d] = %s\n",i,j,column_name[i][j]);
#endif
			for(k=0; k < num_events; k++)
				{
#ifdef DBUG
		fprintf(stderr," column name[%d][%d] = %s, event_list[%d].name = %s ",i,j,column_name[i][j],k,event_list[k].name);
#endif
				if(strcasecmp(column_name[i][j],event_list[k].name) != 0)
					{
					fprintf(stderr,"\n");
					continue;
					}
#ifdef DBUG
				fprintf(stderr," found to be matched\n");
#endif
				fprintf(stderr," column name[%d][%d] = %s, event_list[%d].name = %s ",i,j,column_name[i][j],k,event_list[k].name);
				fprintf(stderr," found to be matched\n");
				fixed_index[l]=num_events*(num_cores + num_sockets) + k;
				event_list[k].fixed = 1;
				}
			j++;
			l++;
			}
		}
			

//	loop over the events, find the fixed events and reset the fixed_index array and count the events to be ordered
//      initialize the high level tree structure

	found_ordered_events = 0;
	for(i=0; i< num_events; i++)if(event_list[i].fixed == 0)found_ordered_events++;
	this_event_order->num_ordered = found_ordered_events;
	this_event_order->num_branch = NUM_BRANCH;
	this_event_order->num_sub_branch = 0;

#ifdef DBUG
	fprintf(stderr,"from init_order, finished loops, ordered event count = %d\n", found_ordered_events);
	for(i=0; i<this_event_order->num_fixed; i++)fprintf(stderr," i= %d, name = %s\n",i,event_list[fixed_index[i]-num_events*(num_cores + num_sockets)].name);
	for(i=0; i<NUM_BRANCH+1; i++)
		{
		j = 0;
		while(j < num_elements[i])
			{
			fprintf(stderr," name [%d][%d] = %s\n",i,j,column_name[i][j]);
			j++;
			}
		}
#endif
//	malloc order buffer then load the fixed events into the first NUM_FIXED slots
//      the order buffer holds the data destined to be printed in the tables
//      making the table printing a simple set of loops over the array with one fprintf per required component

	this_event_order->order = (order_data *)malloc((this_event_order->num_fixed + found_ordered_events )*sizeof(order_data));
	if(this_event_order->order == NULL)
		{
		fprintf(stderr,"failed to malloc order_data array\n");
		err(1,"failed to malloc order array");
		}
	for(i=0; i < this_event_order->num_fixed + found_ordered_events; i++)
		{
		this_event_order->order[k].penalty = 0;
		this_event_order->order[k].cycle = 0;
		}
	global_event_order = this_event_order;
}

event_order_struc_ptr 
set_order_def(int* sample_count)
{
	event_order_struc_ptr this_event_order;
	int i,j,k,l,len, found_fixed_events, found_ordered_events, index, event_list_index;
	index_data * arr;
	uint64_t default_config = 0, default_sample_period = 2000000;
	double default_multiplex = 1.0;

	this_event_order = global_event_order;
#ifdef DBUG
	fprintf(stderr," fixed_index[0] = %d, null_index = %d\n",fixed_index[0],num_events*(num_cores + num_sockets + 1));
	for(i=0; i< num_events + 1; i++)
		{
		fprintf(stderr,"i = %d, multi = %g\n",i,global_multiplex_correction[num_events*(num_cores + num_sockets) + i]);
		if(i < num_events)fprintf(stderr," period = %ld\n",global_attrs[event_list_index].attr.sample.sample_period);
		}
#endif
//      test if the first fixed event, unhalted_core_cycles, was found
//      if so set the default periods and multiplex corrections to those of the cycle event

	if(fixed_index[0] != num_events*(num_cores + num_sockets + 1) )
		{
		default_sample_period = global_attrs[fixed_index[0] - num_events*(num_cores + num_sockets)].attr.sample.sample_period;
		default_multiplex = global_multiplex_correction[fixed_index[0]];
		}
#ifdef DBUG
	fprintf(stderr," in set_order\n");
#endif
//      construct the tree in a nested loop over branches with an inner loop of elements/branch
//      some branch elements have a simple form for their contribution to the branch total like  SUM(events[i][j] * penalty [i][j]
//      some are organized into subcomponents and the subcomponent total in included in the branch sum
//      some events are simply informational
//      some events are in cycles but are actually consistency checks and not included in the branch sum

//	the default tree has the 4 fixed events and nothing else, NUM_BRANCH=0
//	the code below was generated from the wsm-ep code
//	but since there are no defined branches for unknown processors it does almost nothing

	l = 0;
	k = 0;
	for(i=0; i< NUM_BRANCH+1; i++)
		{
#ifdef DBUG
		fprintf(stderr," set_order loop over branches i = %d\n",i);
#endif
		j = 0;
		while(j < num_elements[i])
			{
#ifdef DBUG
		fprintf(stderr," set_order loop over branches i = %d, j = %d\n",i,j);
#endif
			this_event_order->order[k].name = column_name[i][j];
			event_list_index = fixed_index[l] - num_events*(num_cores + num_sockets);
			if(event_list_index < num_events)
				{
//	this is a first group fixed column which was found to be a collected event
				this_event_order->order[k].config = event_list[event_list_index].config;
				this_event_order->order[k].Period = global_attrs[event_list_index].attr.sample.sample_period;
#ifdef DBUG
				fprintf(stderr," i= %d, j= %d, index = %d, list_index = %d LT num_events\n",i,j,fixed_index[l], event_list_index);
#endif
				}
			else
				{
//	this is for fixed events that were not found
				this_event_order->order[k].config = default_config;
				this_event_order->order[k].Period = default_sample_period;
#ifdef DBUG
				fprintf(stderr," i= %d, j= %d, index = %d, list_index = %d GE num_events\n",i,j,fixed_index[l], event_list_index);
#endif
				}
//      common initialization
			this_event_order->order[k].multiplex = global_multiplex_correction[fixed_index[l]];
			this_event_order->order[k].index = fixed_index[l];
			if(i == 0)
				{
//      first fixed event group. cycles, stalls, instructions, uops
//	on the off chance these events just might exist on this processor
				this_event_order->order[k].base_col = j;
				this_event_order->order[k].ctrl_string = (char *)malloc(CTRL_STRING_LEN);
				if(this_event_order->order[k].ctrl_string == NULL)
					{
					fprintf(stderr,"failed to malloc ctrl_string for event %d, %s\n",k,this_event_order->order[index - 1 + j].name);
					err(1,"malloc failed in set_order");
					}
				sprintf(this_event_order->order[k].ctrl_string,":0\0");
				if(k <= 1)
					{
//      first 2 columns are in cycles
					this_event_order->order[k].penalty = 1;
					this_event_order->order[k].cycle = 1;
					}
#ifdef DBUG
		fprintf(stderr," column control %d string = %d%s, index = %d, name = %s\n",k,this_event_order->order[k].base_col,this_event_order->order[k].ctrl_string,this_event_order->order[k].index - num_events*(num_cores + num_sockets ), this_event_order->order[k].name);
#endif
				}
			j++;
			l++;
			k++;
			}
		}
#ifdef DBUG
	fprintf(stderr," number of fixed data columns  k = %d, this_event_order->num_fixed = %d\n",k,this_event_order->num_fixed);
#endif

//	extra events not used in cycle tree are sorted by corrected sample counts
//	load struc with sample_count*multiplex and event index and sort the struc by sample count
	found_ordered_events = this_event_order->num_ordered;

	arr = (index_data *)malloc(found_ordered_events*sizeof(index_data));
	if(arr == NULL)
		{
		fprintf(stderr,"failed to malloc ordered_event sorting array\n");
		err(1,"failed to malloc ordering array");
		}
	j = 0;
	for(i=0; i<num_events; i++)
		{
		if(event_list[i].fixed == 1)continue;
		arr[j].index = i;
		arr[j].val = sample_count[num_events*(num_cores+num_sockets) + i];
		arr[j].val = (int)((double)arr[j].val*global_multiplex_correction[num_events*(num_cores+num_sockets) + i]);
		j++;
		}
//	fprintf(stderr,"finished loop initializing arr[j], final j = %d\n",j);
	if(j != found_ordered_events)
		{
		fprintf(stderr,"miscounted non fixed events, j = %d, found_ordered_events = %d\n",j,found_ordered_events);
		err(1,"micounted ordered events");
		}

//sort the non-fixed/extra/ordered events
	quickSortIndex(arr,found_ordered_events);
//	fprintf(stderr,"returned from quicksort call in set_order\n");
	j = 0;
	index = this_event_order->num_fixed;

	for(i= found_ordered_events - 1; i>=0; i--)
		{
//	initialize the order structure for the events not used in the fixed structures
//	they have been sorted and will be printed out in descending sample count
		this_event_order->order[index + j].name = event_list[arr[i].index].name;
		this_event_order->order[index + j].config = event_list[arr[i].index].config;
		this_event_order->order[index + j].Period = global_attrs[arr[i].index].attr.sample.sample_period;
		this_event_order->order[index + j].multiplex = global_multiplex_correction[num_events*(num_cores+num_sockets) + arr[i].index];
		this_event_order->order[index + j].index = arr[i].index + num_events*(num_cores+num_sockets);
		this_event_order->order[index + j].base_col = num_elements[0] + NUM_BRANCH + j;
		this_event_order->order[index + j].ctrl_string = (char *)malloc(CTRL_STRING_LEN);
		if(this_event_order->order[index + j].ctrl_string == NULL)
			{
			fprintf(stderr,"failed to malloc ctrl_string for event %d, %s\n",k,this_event_order->order[index + j].name);
			err(1,"malloc failed in set_order");
			}
		sprintf(this_event_order->order[index + j].ctrl_string,":0\0");
#ifdef DBUG
		k = index + j;
		fprintf(stderr," column control %d string = %d%s, index = %d, name = %s\n",k,this_event_order->order[k].base_col,this_event_order->order[k].ctrl_string,this_event_order->order[k].index - num_events*(num_cores + num_sockets ), this_event_order->order[k].name);
#endif
		j++;
		}
#ifdef DBUG
	fprintf(stderr," num_fixed = %d, num_ordered = %d\n",this_event_order->num_fixed,this_event_order->num_ordered);
#endif
		
	return this_event_order;
}

void 
branch_eval_def(int* sample_count)
{
	int i,j,k,kk,l,len, branch_order_index, order_index, sub_branch_order_index, num_col,first_event_index;
	event_order_struc_ptr this_event_order;
	order_data * order;

	uint64_t cycle_norm;

	this_event_order = global_event_order;
	order = this_event_order->order;

	first_event_index = num_events*(num_cores + num_sockets);
	branch_order_index = num_events*(num_cores + num_sockets + 1) + 1 ;
	sub_branch_order_index = branch_order_index + NUM_BRANCH;
	order_index = NUM_FIXED;
	cycle_norm = order[0].Period*order[0].multiplex;
#ifdef DBUG
	fprintf(stderr," in branch_eval, global_event_order->order[0].name = %s period = %d, multiplex = %g\n",
		global_event_order->order[0].name,global_event_order->order[0].Period,global_event_order->order[0].multiplex);
	fprintf(stderr," cycle_norm = %ld,order[0].name = %s\n",cycle_norm,order[0].name);
        fprintf(stderr,"global_sample_count totals  ");
        num_col = num_events+global_event_order->num_branch + global_event_order->num_sub_branch;
        for(i=0; i< num_col; i++)fprintf(stderr," %d,",global_sample_count[num_events*(num_cores+num_sockets) + i]);
	fprintf(stderr,"\n lat global_sample_count index = %d, Period = %ld, multi = %g\n",order_index+1,order[order_index+1].Period,order[order_index+1].multiplex);
	kk = 0;
	fprintf(stderr,"global_sample_count branch_order_index = %d, num_elements[%d] = %d\n",branch_order_index,kk,num_elements[kk]);
	kk++;
	if(column_flag == 1)
		{
		fprintf(stderr," before branch_eval computation\n");
		for(i=0; i<this_event_order->num_fixed; i++)
			{
			fprintf(stderr," element %d, name = %s, index-first_index = %d, sample_count[%d] = %d",i,order[i].name,order[i].index-first_event_index,order[i].index,sample_count[order[i].index]);
			if(order[i].index-first_event_index < num_events)
				{
				fprintf(stderr," event_list[%d].name = %s\n",order[i].index-first_event_index,event_list[order[i].index-first_event_index].name);
				}
			else
				{
				fprintf(stderr,"\n");
				}
			}
		}
#endif
#ifdef DBUG

	fprintf(stderr,"leaving branch_eval\n");
	if(column_flag == 1)
		{
		fprintf(stderr," after branch_eval computation\n");
		for(i=0; i<this_event_order->num_fixed; i++)
			{
			fprintf(stderr," element %d, name = %s, index-first_index = %d, sample_count[%d] = %d",i,order[i].name,order[i].index-first_event_index,order[i].index,sample_count[order[i].index]);
			if(order[i].index-first_event_index < num_events)
				{
				fprintf(stderr," event_list[%d].name = %s\n",order[i].index-first_event_index,event_list[order[i].index-first_event_index].name);
				}
			else
				{
				fprintf(stderr,"\n");
				}
			}
		}
#endif
}
