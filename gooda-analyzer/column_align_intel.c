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
#include <stddef.h>
#include <malloc.h>
#include <limits.h>
#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"
#include "gooda_util.h"

FILE *input;
char input_file[PATH_MAX];
order_data* fixed_order_data;
char input_file_snb[]="snb.csv";
char input_file_snb_ep[]="snb_ep.csv";
char input_file_wsm_ep[]="wsm_ep.csv";
char input_file_wsm[]="wsm.csv";
char input_file_ivb[]="ivb.csv";
char input_file_ivb_ep[]="ivb_ep.csv";
char input_file_hsw[]="hsw.csv";
char input_file_hsw_ep[]="hsw_ep.csv";
int *fixed_index;
//	ratio column names
	char rs_empty_duration[]="Avg_RS_empty_duration",wrong_path[]="Wrong_path_cycles";
	char port_saturation[]="port_saturation";
//	event names for basic block execution by LBR analysis
	char inst_retired[]="instruction_retired",inst_retired_prec_dist[]="inst_retired:prec_dist";
	char br_near_taken[]="br_inst_retired:near_taken";
	char lbr_insert_event[]="rob_misc_events:lbr_inserts";
//	this may change in the future based on family/model
int	intel_lbr_entries=16;
int	inst_ret_index=-1, inst_ret_prec_dist_index=-1,br_taken_index=-1;
double	sw_inst_ret_correction = 1.0;
int 	branch_eval_debug=0;

int
read_char_row(uint64_t offset, int num_fixed)
{
	int i,j,k,l,len;
	int retval,field_len, return_val;
	char *lineptr, *temp_buf;
	size_t line_size;
	uint64_t stride;
	char **char_ptr, *tmp_str;
	uint64_t tmp_ptr_addr;

	stride = (uint64_t)sizeof(order_data);
	tmp_ptr_addr = (uint64_t)fixed_order_data;
	tmp_ptr_addr += offset;
	char_ptr = (char**)tmp_ptr_addr;

#ifdef DBUGR
	fprintf(stderr," fixed_order_data address = %p, *char_ptr address = %p, %p\n",fixed_order_data,char_ptr,&(*char_ptr));
	fprintf(stderr," stride = %lx\n",stride);
#endif
//	read the row header first
	lineptr = NULL;
	line_size = 0;
	return_val = getdelim(&lineptr,&line_size,',',input);
	if(return_val == -1)
		err(1,"failed to read second line of %s, line_size = %ld, lineptr = %p, from read_char_row",input_file,line_size,lineptr);
//	if(lineptr)free(lineptr);
//	then loop over the num_fixed fields;
	for(i=0;i<num_fixed; i++)
		{
		if(i < num_fixed -1)return_val = getdelim(&lineptr,&line_size,',',input);
		if(i == num_fixed -1)return_val = getdelim(&lineptr,&line_size,'\n',input);
		if(return_val == -1)
			err(1,"failed to read field %d from char line of %s , line_size = %ld, lineptr = %p, from read_char_row",
				i,input_file,line_size,lineptr);
		field_len = strlen(lineptr);
#ifdef DBUGR
		fprintf(stderr," i = %d, field_len = %d, line_size =%ld\n",i,field_len, line_size);
		fprintf(stderr,"input = \"");
		for(j=0;j<field_len;j++)fprintf(stderr,"%c",lineptr[j]);
		fprintf(stderr,"\"\n");
#endif
		if(field_len <= 3 )
			err(1," string %d had length %d <= 3",i,field_len);
		temp_buf = (char*)malloc(field_len+3); // -3 for the leading and trailing " and then the ,
#ifdef DBUGR
		fprintf(stderr,"returned from malloc\n");
#endif
		if(temp_buf == NULL)
			err(1," failed to malloc name buffer for event %d",i);
//		memcpy(*char_ptr,&lineptr[1],field_len-3);
		for(j=0; j<field_len - 3; j++)temp_buf[j] = lineptr[j+1];
		temp_buf[field_len-3] = '\0';
		*char_ptr = temp_buf;;
#ifdef DBUGR
		fprintf(stderr," output = \"%s\"\n",*char_ptr);
#endif
		tmp_ptr_addr += stride;
		char_ptr = (char**)tmp_ptr_addr;
		}
//	if(lineptr)free(lineptr);
	retval = 0;
	return retval;
}

int
read_int_row(uint64_t offset, int num_fixed)
{
	int i,j,k,l,len;
	int retval,field_len, return_val;
	char *lineptr,*temp_buf;
	size_t line_size;
	uint64_t stride;
	int *int_ptr, *tmp_str;
	uint64_t tmp_ptr_addr;

	stride = (uint64_t)sizeof(order_data);
	tmp_ptr_addr = (uint64_t)fixed_order_data;
	tmp_ptr_addr += offset;
	int_ptr = (int*)tmp_ptr_addr;

#ifdef DBUGR
	fprintf(stderr," fixed_order_data address = %p, int_ptr address = %p, %p\n",fixed_order_data,int_ptr,&(*int_ptr));
	fprintf(stderr," stride = %lx\n",stride);
#endif
//	read the row header first
	lineptr = NULL;
	line_size = 0;
	return_val = getdelim(&lineptr,&line_size,',',input);
	if(return_val == -1)
		err(1,"failed to read first field of int row of %s, line_size = %ld, lineptr = %p, from read_int_row",input_file,line_size,lineptr);
//	if(lineptr)free(lineptr);
//	then loop over the num_fixed fields;
	for(i=0;i<num_fixed; i++)
		{
		if(i < num_fixed -1)return_val = getdelim(&lineptr,&line_size,',',input);
		if(i == num_fixed -1)return_val = getdelim(&lineptr,&line_size,'\n',input);
		if(return_val == -1)
			err(1,"failed to read field %d from char line of %s , line_size = %ld, lineptr = %p, from read_char_row",
				i,input_file,line_size,lineptr);
		field_len = strlen(lineptr);
#ifdef DBUGR
		fprintf(stderr," i = %d, field_len = %d, line_size = %ld\n",i,field_len,line_size);
		fprintf(stderr,"input = ");
		for(j=0;j<field_len;j++)fprintf(stderr,"%c",lineptr[j]);
		fprintf(stderr,"\n");
#endif
		if(field_len >= 2 )
			{
			temp_buf = (char*)malloc(field_len+3);
			for(j=0;j<field_len;j++)temp_buf[j] = lineptr[j];
			temp_buf[field_len] = '\0';
			*int_ptr = atoi(temp_buf);
			free(temp_buf);
			}
		else
			{
			*int_ptr = 0;
			}
#ifdef DBUGR
		fprintf(stderr," output = %d\n",*int_ptr);
#endif
		tmp_ptr_addr += stride;
		int_ptr = (int*)tmp_ptr_addr;
		}
//	if(lineptr)free(lineptr);
	retval = 0;
	return retval;
}

void
init_order_intel(int arch_val)
{


	int i,j,k,l,len, found_fixed_events, found_ordered_events, num_fixed, num_col;
	int num_branch, branch_count, num_sub_branch, sub_branch_count,num_derived, derived_count;
	int retval;
	char mode[]="r";
	char *lineptr;
	size_t line_size;
	ssize_t return_val;
	int field_len;
	event_order_struc_ptr this_event_order;
	uint64_t offset;
	char *file;

	this_event_order = (event_order_struc_ptr)malloc(sizeof(event_order_data));
	if(this_event_order == NULL)
		err(1,"failed to create event_order_struc in init_order_intel");
	fprintf(stderr,"init_order called for arch_val = %d\n",arch_val);
	switch(arch_val)
		{
		case 1:
			file = input_file_wsm_ep;
			break;
		case 4:
			file = input_file_snb;
			break;
		case 5:
			file = input_file_snb_ep;
			break;
		case 6:
			file = input_file_ivb;
			break;
		case 7:
			file = input_file_ivb_ep;
			break;
		case 8:
			file = input_file_hsw;
			break;
		case 9:
			file = input_file_hsw_ep;
			break;
		default:
			err(1," init_order_intel called with invalid value fo arch_val");
			break;
		}

	sprintf(input_file, "%s/report_files/%s", gooda_dir, file);
	input = fopen(input_file, mode);
	if(input == NULL)
		err(1,"failed to open input file %s from init_order_intel",input_file);
	lineptr = NULL;
	line_size = 0;
	return_val = getline(&lineptr,&line_size,input);
	if(return_val == -1)
		err(1,"failed to read first line of %s from init_order_intel",input_file);
//	first line has a : in each event column and this allows the counting of the number of fixed events"
	for(i=0;i<line_size;i++)
		if(lineptr[i] == ':')this_event_order->num_fixed++;
	num_fixed = this_event_order->num_fixed;
	fprintf(stderr," template file = %s, with num_fixed = %d\n",file,num_fixed);

//	if(lineptr)free(lineptr);

	fixed_order_data = (order_data*)malloc(num_fixed*sizeof(order_data));
#ifdef DBUGI
	fprintf(stderr," address of fixed_order_data = %p\n",fixed_order_data);
	fprintf(stderr,"\"column_count\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"\":\",");
	i = num_fixed-1;
	fprintf(stderr,"\":\"");
	fprintf(stderr,"\n");
	
#endif
	if(fixed_order_data == NULL)
		err(1,"failed to malloc fixed_order_data in init_order_intel");
//	start populating the fixed_order_data array
//	second row contains 
//	event, branch, sub_branch and derived column names
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].name -(uint64_t)fixed_order_data);
	retval = read_char_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"names\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"\"%s\",",fixed_order_data[i].name);
	i = num_fixed-1;
	fprintf(stderr,"\"%s\"",fixed_order_data[i].name);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].base_col -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"base_col\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].base_col);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].base_col);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].ctrl_string -(uint64_t)fixed_order_data);
	retval = read_char_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"control_string\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"\"%s\",",fixed_order_data[i].ctrl_string);
	i = num_fixed-1;
	fprintf(stderr,"\"%s\"",fixed_order_data[i].ctrl_string);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].penalty -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"penalties\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].penalty);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].penalty);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].cycle -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"cycles\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].cycle);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].cycle);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].branch -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"branch\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].branch);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].branch);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].sub_branch -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"sub_branch\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].sub_branch);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].sub_branch);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].derived -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"derived\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].derived);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].derived);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].sum -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"sum\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].sum);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].sum);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].sub_branch_sum -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"sub_branch_sum\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].sub_branch_sum);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].sub_branch_sum);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].ratio -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"ratio\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].ratio);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].ratio);
	fprintf(stderr,"\n");
#endif
	offset = (uint64_t) ((uint64_t)&fixed_order_data[0].num_branch -(uint64_t)fixed_order_data);
	retval = read_int_row(offset, num_fixed);
#ifdef DBUGI
	fprintf(stderr,"\"num_branch\",");
	for(i=0;i<num_fixed-1;i++)fprintf(stderr,"%d,",fixed_order_data[i].num_branch);
	i = num_fixed-1;
	fprintf(stderr,"%d",fixed_order_data[i].num_branch);
	fprintf(stderr,"\n");
#endif

#ifdef DBUGI
	fprintf(stderr,"fixed assignments finished in init_order\n");
#endif

	for(i=0; i< num_events; i++)event_list[i].fixed = 0;

	this_event_order->num_fixed = num_fixed;
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

#ifdef DBUGI
	fprintf(stderr,"init order, about to start triple loop to find fixed events\n");
#endif

//      loop over the branches and the components of each branch and for each branch component 
//      find the matching event name in the perf.data header data structure array, event_list[k]

	l = 0;
	for(i=0; i<num_fixed; i++)
		{
#ifdef DBUGII
			fprintf(stderr," name [%d] = %s\n",i,fixed_order_data[i].name);
#endif
			for(k=0; k < num_events; k++)
				{
#ifdef DBUGII
		fprintf(stderr," fixed_order_data[%d].name = %s, event_list[%d].name = %s ",i,fixed_order_data[i].name,k,event_list[k].name);
#endif
				if(strcasecmp(fixed_order_data[i].name,event_list[k].name) != 0)
					{
#ifdef DBUGII
					fprintf(stderr,"\n");
#endif
					continue;
					}
#ifdef DBUGII
				fprintf(stderr," found to be matched\n");
#endif
//				fprintf(stderr," fixed_order_data[%d].name = %s, event_list[%d].name = %s ",
//					i,fixed_order_data[i].name,k,event_list[k].name);
//				fprintf(stderr," found to be matched\n");
				fixed_index[i]=num_events*(num_cores + num_sockets) + k;
				event_list[k].fixed = 1;
				}
		}
		
//	fprintf(stderr,"finished nested loop\n");	

//	loop over the events and count the events to be ordered
//	initialize the high level tree structure

	found_ordered_events = 0;
	for(i=0; i< num_events; i++)if(event_list[i].fixed == 0)
		{
		found_ordered_events++;
//		fprintf(stderr," ordered_event_name = %s\n",event_list[i].name);
		}
//	fprintf(stderr,"initialized event_list\n");	
	this_event_order->num_ordered = found_ordered_events;
	this_event_order->num_branch=0;
	this_event_order->num_sub_branch=0;
	this_event_order->num_derived=0;
	for(i=0; i<num_fixed; i++)
		{
		if(fixed_order_data[i].branch == 1)this_event_order->num_branch++;
		if(fixed_order_data[i].derived == 1)this_event_order->num_derived++;
		if(fixed_order_data[i].sub_branch == 1)this_event_order->num_sub_branch++;
		}
//	fprintf(stderr,"initialized branch,derived,sub_branch\n");	
	num_branch = this_event_order->num_branch;
	num_derived = this_event_order->num_derived;
	num_sub_branch = this_event_order->num_sub_branch;
	branch_count = 0;
	sub_branch_count = 0;
	derived_count = 0;

#ifdef DBUGI
	fprintf(stderr,"from init_order, finished nested loops, ordered event count = %d\n", found_ordered_events);
	fprintf(stderr," num_fixed = %d, num_branch = %d, num_sub_branch = %d, num_derived = %d\n",num_fixed,num_branch,num_sub_branch,num_derived);
	for(i=0; i<this_event_order->num_fixed; i++)
		if(event_list[fixed_index[i]-num_events*(num_cores + num_sockets)].fixed ==1 )
			{
			fprintf(stderr," i= %d, name = %s\n",i,event_list[fixed_index[i]-num_events*(num_cores + num_sockets)].name);
			}
		else if((fixed_order_data[i].branch == 0 ) && (fixed_order_data[i].sub_branch == 0) && (fixed_order_data[i].derived == 0))
			{
			fprintf(stderr," event i = %d, %s, was not found\n",i,fixed_order_data[i].name);
			}
	for(i=0; i<num_fixed; i++)
		{
		fprintf(stderr," fixed_order_data[%d].name = %s\n",i,fixed_order_data[i].name);
		}
	fprintf(stderr,"finished name loop\n");
#endif

//	malloc order buffer array then initialize the penalty and cycle flags to zero for the fixed and ordered elements
//	the order buffer holds the data destined to be printed in the tables
//	making the table printing a simple set of loops over the array with one fprintf per required component

	num_col = this_event_order->num_fixed + found_ordered_events;
//	fprintf(stderr," before malloc this_event_order-> order of size num_col = %d\n",num_col);
//	fprintf(stderr," num_fixed = %d, found_ordered_events = %d, num_branch = %d, num_sub_branch = %d, num_derived = %d\n",
//		num_fixed,found_ordered_events,num_branch,num_sub_branch,num_derived);
//	fprintf(stderr," num_events = %d, num_cores = %d, num_sockets = %d\n",num_events,num_cores,num_sockets);
	this_event_order->order = (order_data *)malloc(num_col*sizeof(order_data));
	if(this_event_order->order == NULL)
		{
		fprintf(stderr,"failed to malloc order_data array\n");
		err(1,"failed to malloc order array");
		}
//	fprintf(stderr," malloced this_event_order-> order of size %d\n",this_event_order->num_fixed + found_ordered_events);
	for(i=0; i < this_event_order->num_fixed; i++)
		{
//		fprintf(stderr," initializing order [%d]\n",i);
		this_event_order->order[i].name = fixed_order_data[i].name;
		this_event_order->order[i].ctrl_string = fixed_order_data[i].ctrl_string;
		this_event_order->order[i].base_col = fixed_order_data[i].base_col;
		this_event_order->order[i].penalty = fixed_order_data[i].penalty;
		this_event_order->order[i].cycle = fixed_order_data[i].cycle;
		this_event_order->order[i].branch = fixed_order_data[i].branch;
		this_event_order->order[i].sub_branch = fixed_order_data[i].sub_branch;
		this_event_order->order[i].derived = fixed_order_data[i].derived;
		this_event_order->order[i].sum = fixed_order_data[i].sum;
		this_event_order->order[i].sub_branch_sum = fixed_order_data[i].sub_branch_sum;
		this_event_order->order[i].ratio = fixed_order_data[i].ratio;
		this_event_order->order[i].num_branch = fixed_order_data[i].num_branch;
		this_event_order->order[i].index = fixed_index[i];
		if(fixed_order_data[i].branch == 1)
			{
			this_event_order->order[i].index = num_events*(num_cores + num_sockets + 1) + 1 + branch_count;
			branch_count++;
			}
		if(fixed_order_data[i].sub_branch == 1)
			{
			this_event_order->order[i].index = num_events*(num_cores + num_sockets + 1) + 1 + num_branch + sub_branch_count;
			sub_branch_count++;
			}
		if(fixed_order_data[i].derived == 1)
			{
			this_event_order->order[i].index = num_events*(num_cores + num_sockets + 1) + 1 + num_branch + num_sub_branch + derived_count;
			derived_count++;
			}
		}
//	fprintf(stderr,"initialized order for fixed events\n");	
	for(i=this_event_order->num_fixed; i < this_event_order->num_fixed + found_ordered_events; i++)
		{
		this_event_order->order[i].penalty = 0;
		this_event_order->order[i].cycle = 0;
		this_event_order->order[i].derived = 0;
		this_event_order->order[i].branch = 0;
		this_event_order->order[i].sub_branch = 0;
		this_event_order->order[i].sum = 0;
		this_event_order->order[i].sub_branch_sum = 0;
		this_event_order->order[i].ratio = 0;
		this_event_order->order[i].num_branch = 0;
		}
//	fprintf(stderr,"initialized order for ordered events\n");	

//	derived events are architecture specific
//	snb specific for the next 15 lines
	if(arch_val >= 4)
		{
		derived_events = (derived_sample_data*)malloc(num_derived*sizeof(derived_sample_data));
		if(derived_events == NULL)
			err(1," init_order_intel failed to malloc buffer for derived_events");

//	define the positions of the derived events
		j = 0;
		for(i=0; i< num_fixed; i++)
			{
			if(fixed_order_data[i].derived == 1)
				{
				derived_events[j].table_position = i;
				j++;
				}
			}
//	this sets up the assorted global indecies for the source, target, bb_exec, sw_inst_ret and next_taken derived LBR data
		derived_events[0].sample_index = num_events*(num_cores + num_sockets + 1) + num_branch + num_sub_branch + 1;
		for(i=1;i<num_derived;i++)
			derived_events[i].sample_index = derived_events[0].sample_index + i;
		source_index = derived_events[0].sample_index;
		source_column = derived_events[0].table_position;
		target_index = derived_events[1].sample_index;
		target_column = derived_events[1].table_position;
		bb_exec_index = derived_events[2].sample_index;
		bb_exec_column = derived_events[2].table_position;
		sw_inst_retired_index = derived_events[3].sample_index;
		sw_inst_retired_column = derived_events[3].table_position;
		next_taken_index = derived_events[4].sample_index;
		next_taken_column = derived_events[4].table_position;
		}
// end of snb specific code

#ifdef DBUGI
	fprintf(stderr," init_order_intel, num_fixed = %d, num_ordered = %d\n",this_event_order->num_fixed,this_event_order->num_ordered);
	fprintf(stderr," init_order_intel, event_indecies = %d, num_branch = %d, num_sub_branch = %d, num_derived = %d\n",
		num_events*(num_cores + num_sockets + 1),this_event_order->num_branch,this_event_order->num_sub_branch,this_event_order->num_derived);
	fprintf(stderr," init_order_intel, source_index = %d, target_index = %d\n",source_index,target_index);
#endif

	global_event_order = this_event_order;
}

#define CTRL_STRING_LEN 6

event_order_struc_ptr 
set_order_intel(int* sample_count, int arch_val)
{
	event_order_struc_ptr this_event_order;
	int i,j,k,l,m,len, found_fixed_events, found_ordered_events, index, event_list_index;
	index_data * arr;
	uint64_t default_config = 0, default_sample_period = 2000000;
	double default_multiplex = 1.0;
	char uops_issued[]="uops_issued:any",uops_issued_cycles[]="uops_issued:any:c=1";
	int base_index, first_ordered_col;
	int preferred_inst_retired=0;
//	CAUTION*****CAUTION******CAUTION
//	MANY LATENCIES ARE FREQUENCY DEPENDENT
//	YOU SHOULD DETERMINE THE CORRECT VALUES FOR YOUR MACHINE WITH KERNELS
//	AND CORRECT THE CONSTANTS BELOW AS NEEDED

	int cycles, uops_issued_index=-1, uops_issued_cycles_index = -1;

	base_index = num_events*(num_cores + num_sockets);
	this_event_order = global_event_order;
	default_sample_period = 2000000;
	default_multiplex = num_events/4 + 1;
//	initialize fixed list in case user was delusional and thought they knew what they were doing
	for(i = 0; i< this_event_order->num_fixed; i++)
		{
		this_event_order->order[i].Period = default_sample_period;
		this_event_order->order[i].multiplex = default_multiplex;
		}

#ifdef DBUGI
	fprintf(stderr," fixed_index[0] = %d, null_index = %d\n",fixed_index[0],num_events*(num_cores + num_sockets + 1));
	fprintf(stderr," num_events = %d\n",num_events);
	for(i=0; i< num_events + 1; i++)
		{
		fprintf(stderr,"i = %d, multi = %g\n",i,global_multiplex_correction[num_events*(num_cores + num_sockets) + i]);
		if(i < num_events)fprintf(stderr," period = %ld\n",
			global_attrs[num_events*(num_cores + num_sockets) + i].attr.sample.sample_period);
		}
#endif
//	test if the first fixed event, unhalted_core_cycles, was found
//	if so set the default periods and multiplex corrections to those of the cycle event

	if(fixed_index[0] != num_events*(num_cores + num_sockets + 1) )
		{
		default_sample_period = global_attrs[fixed_index[0] - num_events*(num_cores + num_sockets)].attr.sample.sample_period;
		default_multiplex = global_multiplex_correction[fixed_index[0]];
		}
#ifdef DBUGI
	fprintf(stderr,"set_order: setting period, multiplex and config\n");
#endif
	for(i=0; i<this_event_order->num_fixed; i++)
		{
		if(this_event_order->order[i].index < num_events*(num_cores + num_sockets + 1) )
			{
			j = this_event_order->order[i].index - base_index;
			this_event_order->order[i].config = event_list[j].config;
			this_event_order->order[i].multiplex = global_multiplex_correction[this_event_order->order[i].index];
			this_event_order->order[i].Period = event_list[j].period;
//	basic block specific
			if(strcmp(this_event_order->order[i].name,inst_retired_prec_dist) == 0)
				{
				inst_ret_prec_dist_index = i;
				preferred_inst_retired = 1;
				if(sw_inst_retired_index != 0)
					{
					this_event_order->order[sw_inst_retired_index].config = 0;
					this_event_order->order[sw_inst_retired_index].Period = this_event_order->order[i].Period;
					this_event_order->order[sw_inst_retired_index].multiplex = this_event_order->order[i].multiplex;
					}
				}
			if(strcmp(this_event_order->order[i].name,inst_retired) == 0)
				{
				inst_ret_index = i;
				if(preferred_inst_retired == 0)
					{
					if(sw_inst_retired_index != 0)
						{
						this_event_order->order[sw_inst_retired_index].config = 0;
						this_event_order->order[sw_inst_retired_index].Period = this_event_order->order[i].Period;
						this_event_order->order[sw_inst_retired_index].multiplex = this_event_order->order[i].multiplex;
						}
					}
				}
			if(strcmp(this_event_order->order[i].name,br_near_taken) == 0)
				br_taken_index = i;
			if(strcmp(this_event_order->order[i].name,lbr_insert_event) == 0)
				br_taken_index = i;
			}
		else if(i != sw_inst_retired_index)
			{
			this_event_order->order[i].config = 0;
			this_event_order->order[i].multiplex = default_multiplex;
			this_event_order->order[i].Period = default_sample_period;
//	snb specific
			if(strcmp(this_event_order->order[i].name,rs_empty_duration) == 0)
				{
				this_event_order->order[i].multiplex = 1.0;
				this_event_order->order[i].Period = 1;
				}
			}
		}
// set corrections for SW_inst_retired & BB_exec so they are normalized to the prefered inst_retired event
//	make sure sw_inst_retired constants are actually set
	if(sw_inst_retired_index != 0)
		{
		if(this_event_order->order[sw_inst_retired_index].Period == 0)
			{
			this_event_order->order[sw_inst_retired_index].config = 0;
			this_event_order->order[sw_inst_retired_index].Period = default_sample_period;
			this_event_order->order[sw_inst_retired_index].multiplex = default_multiplex;
			}
		else if(br_taken_index > 0)
			{
			sw_inst_ret_correction = ( (double)this_event_order->order[br_taken_index].Period *
				(double)this_event_order->order[br_taken_index].multiplex) /
				( (double)this_event_order->order[sw_inst_retired_index].Period *
				  (double)this_event_order->order[sw_inst_retired_index].multiplex);
			sw_inst_ret_correction = sw_inst_ret_correction/( (double)(intel_lbr_entries - 1));
			}
		}
	if(bb_exec_index != 0)
		{
		this_event_order->order[bb_exec_index].config = 0;
		this_event_order->order[bb_exec_index].Period = this_event_order->order[sw_inst_retired_index].Period;
		this_event_order->order[bb_exec_index].multiplex = this_event_order->order[sw_inst_retired_index].multiplex;
		}

#ifdef DBUGI
	fprintf(stderr," in set_order, setting uop_issue_rate\n");
#endif
	for(i=0; i<this_event_order->num_fixed; i++)
		{
		if(strcmp(this_event_order->order[i].name,uops_issued) == 0)
			uops_issued_index = this_event_order->order[i].index;
		if(strcmp(this_event_order->order[i].name,uops_issued_cycles) == 0)
			uops_issued_cycles_index = this_event_order->order[i].index;
		}
	if((sample_count[uops_issued_index] >0) && (sample_count[uops_issued_cycles_index] >0))
		uop_issue_rate =(double) sample_count[uops_issued_index]/(double)sample_count[uops_issued_cycles_index];
#ifdef DBUGI
	fprintf(stderr,"this_event_order->num_fixed = %d, uop_issue_rate = %g\n",this_event_order->num_fixed,uop_issue_rate);
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
		err(1,"miscounted ordered events");
		}

//sort the non-fixed/extra/ordered events
	quickSortIndex(arr,found_ordered_events);
//	fprintf(stderr,"returned from quicksort call in set_order\n");
	j = 0;
	first_ordered_col = fixed_order_data[this_event_order->num_fixed-1].base_col + 1;
	index = this_event_order->num_fixed;

	for(i= found_ordered_events - 1; i>=0; i--)
		{
//      initialize the order structure for the events not used in the fixed structures
//      they have been sorted and will be printed out in descending sample count
		this_event_order->order[index + j].name = event_list[arr[i].index].name;
		this_event_order->order[index + j].config = event_list[arr[i].index].config;
		this_event_order->order[index + j].Period = global_attrs[arr[i].index].attr.sample.sample_period;
		this_event_order->order[index + j].multiplex = global_multiplex_correction[num_events*(num_cores+num_sockets) + arr[i].index];
		this_event_order->order[index + j].index = arr[i].index + num_events*(num_cores+num_sockets);
		this_event_order->order[index + j].base_col = first_ordered_col + j;
		this_event_order->order[index + j].ctrl_string = (char *)malloc(CTRL_STRING_LEN);
		if(this_event_order->order[index + j].ctrl_string == NULL)
			{
			fprintf(stderr,"failed to malloc ctrl_string for event %d, %s\n",k,this_event_order->order[index + j].name);
			err(1,"malloc failed in set_order");
			}
		sprintf(this_event_order->order[index + j].ctrl_string,":0\0");
#ifdef DBUGI
		k = index + j;
		fprintf(stderr," column control %d string = %d%s, index = %d, name = %s\n",
			k,this_event_order->order[k].base_col,this_event_order->order[k].ctrl_string,
			this_event_order->order[k].index - num_events*(num_cores + num_sockets ), this_event_order->order[k].name);
#endif
		j++;
		}
#ifdef DBUGI
	fprintf(stderr," num_fixed = %d, num_ordered = %d\n",this_event_order->num_fixed,this_event_order->num_ordered);
#endif
		
	return this_event_order;
}

int eval_flag=0;
void 
branch_eval_intel(int* sample_count)
{
	int i,j,k,kk,l,len, branch_order_index, sub_branch_order_index, num_col,first_event_index;
	event_order_struc_ptr this_event_order;
	order_data * order;
	int  cycles, num_fixed, num_ordered, ratio;
	double ww1,ww2,ww3,ww4;

	int port_thresh=80, port_mult=100, port_max, port_val;
	uint64_t cycle_norm;

	this_event_order = global_event_order;
	order = this_event_order->order;

	first_event_index = num_events*(num_cores + num_sockets);
	num_fixed = global_event_order->num_fixed;
	num_ordered = global_event_order->num_ordered;

	if(order[0].index >= num_events*(num_cores + num_sockets + 1))
		{
#ifdef DBUG
		fprintf(stderr,"incomplete cycle tree no unhalted_core_cycle event\n");
#endif
		}
	cycle_norm = order[0].Period*order[0].multiplex;
#ifdef DBUG
	fprintf(stderr," in branch_eval, global_event_order->order[0].name = %s period = %lu, multiplex = %g\n",
		global_event_order->order[0].name,global_event_order->order[0].Period,global_event_order->order[0].multiplex);
	fprintf(stderr," cycle_norm = %ld,order[0].name = %s\n",cycle_norm,order[0].name);
        fprintf(stderr,"global_sample_count totals  ");
        num_col = num_events+global_event_order->num_branch + global_event_order->num_sub_branch;
        for(i=0; i< num_col; i++)fprintf(stderr," %d,",global_sample_count[num_events*(num_cores+num_sockets) + i]);
	if(branch_eval_debug == 0)
		{
		fprintf(stderr," order_array\n");
		for(i=0; i<num_fixed; i++)
			{
			fprintf(stderr," i = %d, index = %d, name = %s, period = %ld, multiplex = %5.4lf\n",
				i,order[i].index, order[i].name, order[i].Period, order[i].multiplex);
			}
		branch_eval_debug = 1;
		}
#endif
	j = 0;
	k = 0;

	for(i=0; i<num_fixed; i++)
		{
		ratio = order[i].ratio;
		if(order[i].branch == 1)
			{
			branch_order_index = order[i].index;
			if(i != bb_exec_index)sample_count[branch_order_index] = 0;
			}
		if(order[i].sub_branch == 1)
			{
			sub_branch_order_index = order[i].index;
			sample_count[sub_branch_order_index] = 0;
			}
		if((order[i].sum == 1) && (ratio != 1))
			{
			ww1 = (double)order[i].Period*(double)order[i].multiplex/cycle_norm;
			sample_count[branch_order_index]+=(int) ( (double)sample_count[order[i].index]*(double)order[i].penalty*ww1);
			}
		if((order[i].sub_branch_sum == 1) && (ratio != 1))
			{
			ww1 = (double)order[i].Period*(double)order[i].multiplex/cycle_norm;
			sample_count[sub_branch_order_index]+=(int) ( (double)sample_count[order[i].index]*(double)order[i].penalty*ww1);
			}
		if(order[i].index == bb_exec_index)
			sample_count[order[i].index] = (int)((double)sample_count[order[i].index]*sw_inst_ret_correction);
		if(order[i].index == sw_inst_retired_index)
			sample_count[order[i].index] = (int)((double)sample_count[order[i].index]*sw_inst_ret_correction);

		if(ratio == 1)
			{
//			this is still old fashioned sausage making

			if(strcmp(order[i].name,rs_empty_duration) == 0)
				{
//				rs_empty duration
				ww1 = (double)sample_count[order[i-2].index]*(double)order[i-2].Period*order[i-2].multiplex/cycle_norm;
				ww2 = (double)sample_count[order[i-1].index]*(double)order[i-1].Period*order[i-1].multiplex/cycle_norm;
				if(ww2 > 0.)sample_count[order[i].index] = (int)(ww1/ww2);
				}
			else if(strcmp(order[i].name,wrong_path) == 0)
				{
//				wasted work/wrong path uop flow
				ww1 = (double)sample_count[order[i+1].index]*(double)order[i+1].Period*order[i+1].multiplex/cycle_norm;
				ww2 = (double)sample_count[order[i+2].index]*(double)order[i+2].Period*order[i+2].multiplex/cycle_norm;
				sample_count[order[i].index] = (int)(ww1 - ww2)/uop_issue_rate;
				if(sample_count[order[i].index] < 0)sample_count[order[i].index] = 0;
				sample_count[branch_order_index]+=sample_count[order[i].index];
				}
			else if(strcmp(order[i].name,port_saturation) == 0)
				{
//	port saturation
#ifdef DBUG
				fprintf(stderr,"starting port saturation\n");
#endif
				port_max = 0;
				for(k=0; k < 6; k++)
					{
					port_val = (int) ((double)order[i+k+1].Period*(double)order[i+k+1].multiplex/cycle_norm)*
							(double)sample_count[order[i+k+1].index];
#ifdef DBUG
				fprintf(stderr,"port index = %d\n",order[k+1+i].index-first_event_index);
#endif
					if(port_max < port_val)port_max = port_val;
					}
				cycles = sample_count[order[0].index];
				if(cycles <= 0) cycles=1000000000;
				if(port_mult*port_val/cycles > port_thresh)sample_count[order[i].index] = port_val;
				eval_flag = 1;
				}
			else
				{
				err(1,"unknown ratio column %d name = %s\n",i,order[i].name);
				}
			}
		}		
#ifdef DBUG
	fprintf(stderr,"leaving branch_eval\n");
	if(column_flag == 1)
		{
		fprintf(stderr," after branch_eval computation\n");
		for(i=0; i<this_event_order->num_fixed; i++)
			{
			fprintf(stderr," element %d, name = %s, index-first_index = %d, sample_count[%d] = %d",
				i,order[i].name,order[i].index-first_event_index,order[i].index,sample_count[order[i].index]);
			if(order[i].index-first_event_index < num_events)
				{
				fprintf(stderr," event_list[%d].name = %s\n",order[i].index-first_event_index,
					event_list[order[i].index-first_event_index].name);
				}
			else
				{
				fprintf(stderr,"\n");
				}
			}
		}
#endif
}
