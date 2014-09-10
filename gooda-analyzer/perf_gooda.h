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

#include <sys/types.h>
#include "perf_event.h"

#define u64max 0xFFFFFFFFFFFFFFFF

extern uint64_t tsc_now, this_time;

typedef struct attrs_struc * attrs_struc_ptr;
//typedef struct mmap_struc * mmap_struc_ptr;
typedef struct lost_struc * lost_struc_ptr;
//typedef struct comm_struc * comm_struc_ptr;
typedef struct exit_struc * exit_struc_ptr;
typedef struct throttle_struc * throttle_struc_ptr;
typedef struct unthrottle_struc * unthrottle_struc_ptr;
typedef struct fork_struc * fork_struc_ptr;
typedef struct read_struc * read_struc_ptr;
typedef struct raw_sample_struc * raw_sample_struc_ptr;
typedef struct buildid_struc * buildid_struc_ptr;
typedef struct pmu_programming_struc * pmu_programming_struc_ptr;
typedef struct event_name_struc * event_name_struc_ptr;
typedef struct event_order_struc * event_order_struc_ptr;
typedef struct order_struc * order_struc_ptr;


typedef struct attrs_struc{
	attrs_struc_ptr next;
	attrs_struc_ptr previous;
	uint64_t	perf_evt_sel;
	uint32_t	id;
	uint32_t	type;
	uint32_t	period;
	}attrs_data;

typedef struct mmap_struc{
	mmap_struc_ptr	next;
	mmap_struc_ptr	previous;
	char*		filename;
	uint64_t	addr;
	uint64_t	len;
	uint64_t	time;
	module_struc_ptr this_module;
	process_struc_ptr this_process;
	process_struc_ptr principal_process;
	char *		buildid;
	uint32_t	pid;
	uint32_t	tid;
	uint64_t	pgoff;
	uint64_t	tsc_start;
	uint64_t	tsc_end;
	uint64_t	last_pgoff;
	int		is_kernel;
	}mmap_data;

typedef struct lost_struc{
	lost_struc_ptr	next;   
	lost_struc_ptr	previous;
	uint64_t	time;
	uint64_t	id;
	uint64_t	lost;
	}lost_data;

typedef struct comm_struc{
	comm_struc_ptr	next;   
	comm_struc_ptr	previous;
	mmap_struc_ptr	first_mmap;
	char*		name;
	uint64_t	time;
	uint32_t	pid;
	uint32_t	tid;
	}comm_data;

typedef struct exit_struc{
	exit_struc_ptr	next;   
	exit_struc_ptr	previous;
	uint64_t	time;
	uint32_t	pid;
	uint32_t	ppid;
	uint32_t	tid;
	uint32_t	ptid;
	}exit_data;

typedef struct throttle_struc{
	throttle_struc_ptr	next;   
	throttle_struc_ptr	previous;
	uint64_t	time;
	uint64_t	id;
	uint64_t	stream_id;
	}throttle_data;

typedef struct unthrottle_struc{
	unthrottle_struc_ptr	next;   
	unthrottle_struc_ptr	previous;
	uint64_t	time;
	uint64_t	id;
	uint64_t	stream_id;
	}unthrottle_data;

typedef struct fork_struc{
	uint32_t	pid;
	uint32_t	ppid;
	uint32_t	tid;
	uint32_t	ptid;
 	uint64_t	time;
	}fork_data;

typedef struct read_struc{
	read_struc_ptr	next;   
	read_struc_ptr	previous;
	char*		buffer;
	uint64_t	time;
	uint32_t	size;
	}read_data;

typedef struct raw_sample_struc{
	raw_sample_struc_ptr	next;
	raw_sample_struc_ptr	previous;
	uint64_t	time;
	uint64_t	ip;
	uint64_t	pebs_ip;
	uint64_t	str_id;
	uint64_t	sample_period;
	uint32_t	pid;
	uint32_t	tid;
	uint32_t	cpu;
	}raw_sample_data;

typedef struct pmu_programming_struc{
	sample_struc_ptr	next;   
	sample_struc_ptr	previous;
	uint64_t	str_id;
	uint32_t	cpu;
	}pmu_programming_data;

typedef struct event_name_struc{
	char *		name;
	uint64_t	config;
	uint64_t 	period;
	uint64_t	branch_sample_type;
	int		return_filtered_return;
	int		near_taken_filtered_any_taken;
	int		fixed;
	} event_name_data;

typedef struct order_struc{
	char *		name;
	char*		ctrl_string;
	uint64_t	config;
	uint64_t	Period;
	double		multiplex;
	int		index;
	int 		base_col;
	int		penalty;
	int		cycle;
	int		branch;
	int		sub_branch;
	int		derived;
	int		sum;
	int		sub_branch_sum;
	int		ratio;
	int		num_branch;
	}order_data;

typedef struct event_order_struc{
	order_data *	order;
	int		num_fixed;
	int		num_ordered;
	int		num_branch;
	int		num_sub_branch;
	int		num_derived;
	}event_order_data;

typedef struct derived_event_struc{
	int		table_position;
	int		sample_index;
	}derived_sample_data;

extern mmap_struc_ptr  mmap_stack, mmap_current, kernel_mmap, previous_mmap;
extern comm_struc_ptr	comm_stack, comm_current;
extern int global_flag1,global_flag2;
extern double sqrt_five, max_entry_fraction, sum_cutoff;
extern event_name_struc_ptr event_list;
extern event_order_struc_ptr global_event_order;
extern derived_sample_data* derived_events;
extern int debug_flag;

mmap_struc_ptr mmap_struc_create();
lost_struc_ptr lost_struc_create();
comm_struc_ptr comm_struc_create();
exit_struc_ptr exit_struc_create();
throttle_struc_ptr throttle_struc_create();
unthrottle_struc_ptr unthrottle_struc_create();
fork_struc_ptr fork_struc_create();
read_struc_ptr read_struc_create();
raw_sample_struc_ptr raw_sample_struc_create();
pmu_programming_struc_ptr pmu_programming_struc_create();
event_order_struc_ptr set_order(int* sample_count);
event_order_struc_ptr set_order_intel(int* sample_count, int arch_val);
event_order_struc_ptr set_order_wsm(int* sample_count);
event_order_struc_ptr set_order_snb(int* sample_count);
event_order_struc_ptr set_order_def(int* sample_count);
void init_order_intel(int arch_val);
void init_order();
void branch_eval(int* sample_count);
