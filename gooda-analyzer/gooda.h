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

//     header files for Gooda
//     Google Optimization Data Analyzer
//     Dispencer of wisdom and Insight
//     aka   DWI

#include <sys/types.h>
#include <inttypes.h>


typedef struct function_struc * function_struc_ptr;
typedef struct source_struc * source_struc_ptr;
typedef struct principal_file_struc * principal_file_struc_ptr;
typedef struct module_struc * module_struc_ptr;
typedef struct thread_struc * thread_struc_ptr;
typedef struct process_struc * process_struc_ptr;
typedef struct child_struc * child_struc_ptr;
typedef struct sample_struc * sample_struc_ptr;
typedef struct branch_struc * branch_struc_ptr;
typedef struct branch_site_struc * branch_site_struc_ptr;
typedef struct func_branch_struc * func_branch_struc_ptr;
typedef struct call_chain_struc * call_chain_struc_ptr;
typedef struct hash_struc * hash_struc_ptr;
typedef struct rva_hash_struc * rva_hash_struc_ptr;
typedef struct function_location * function_location_ptr;
typedef struct function_location_stack * function_location_stack_ptr;
typedef struct functionlist_struc * functionlist_struc_ptr;
typedef struct asm_struc * asm_struc_ptr;
typedef struct basic_block_struc * basic_block_struc_ptr;
typedef struct source_line_struc * source_line_struc_ptr;
typedef struct file_list_struc * file_list_struc_ptr;
typedef struct addr_list_struc * addr_list_struc_ptr;

typedef struct mmap_struc * mmap_struc_ptr;
typedef struct comm_struc * comm_struc_ptr;

typedef struct pointer_struc * pointer_struc_ptr;
typedef struct pointer_struc{
        sample_struc_ptr        ptr;
        uint64_t                val;
        }pointer_data;


typedef struct function_struc{
	function_struc_ptr	next;
	function_struc_ptr	previous;
	func_branch_struc_ptr	sources;
	func_branch_struc_ptr	targets;
	sample_struc_ptr	first_rva;
	asm_struc_ptr		first_asm;
	basic_block_struc_ptr	first_bb;
	source_line_struc_ptr	first_source_line;
	char*			function_name;
	char*			function_mangled_name;
	uint64_t		function_rva_start;
	uint64_t		function_length;
	source_struc_ptr	source_file;
	principal_file_struc_ptr	principal_file;
	file_list_struc_ptr	principal_file_list;
	module_struc_ptr	this_module;
	process_struc_ptr	this_process;
	int*			sample_count;
	void *			sample_order;
	int			cycle_count;
	int			inst_count;
	int			total_sample_count;
	int			total_branches;
	int			total_targets;
	int			total_sources;
	int			func_sources;
	int			func_targets;
	int			funclist_index;
	int			called_branch_eval;
	}function_data;

typedef struct source_struc{
	source_struc_ptr	next;
	source_struc_ptr	previous;
	char*			source_file;
	char*			source_name;
	char*			path;
	int*			sample_count;
	int*			sample_order;
	int			cycle_count;
	int			inst_count;
	int			total_sample_count;
	}source_data;

typedef struct principal_file_struc{
	principal_file_struc_ptr	next;
	principal_file_struc_ptr	previous;
	char*			principal_file_name;
	char*			path;
	char*			compile_options;
	int*			sample_count;
	int*			sample_order;
	int			cycle_count;
	int			inst_count;
	int			total_sample_count;
	}principal_file_data;

typedef struct module_struc{
	module_struc_ptr	next;
	module_struc_ptr	previous;
	rva_hash_struc_ptr	this_table;
	sample_struc_ptr	first_sample;
	pointer_data 		* rva_list;
	function_struc_ptr	first_function;
	function_struc_ptr	current_function;
	functionlist_struc_ptr	function_list;
	source_struc_ptr	first_source;
	source_struc_ptr	current_source;
	char*			module_name;
	char*			path;
	char*			local_path;
	char*			buildid;
	uint64_t		starting_ip;
	uint64_t		length;
	uint64_t		time;
	uint64_t		load_tsc;
	uint64_t		unload_tsc;
	int*			sample_count;
	int*			sample_order;
	int			cycle_count;
	int			inst_count;
	int			total_sample_count;
	int			rva_count;
	int			total_branches;
	int			total_sources;
	int			total_targets;
	int			local_calls;
	int			remote_calls;
	int			set_starting_ip;
	int			bin_type;
	int			is_kernel;
	}module_data;

typedef struct thread_struc{
	thread_struc_ptr	next;
	thread_struc_ptr	previous;
	int*			sample_count;
	int*			sample_order;
	uint32_t		tid;
	int			cycle_count;
	int			inst_count;
	int			total_sample_count;
	}thread_data;


typedef struct process_struc{
	process_struc_ptr	next;
	process_struc_ptr	previous;
	process_struc_ptr	principal_next;
	process_struc_ptr	principal_previous;
	process_struc_ptr	principal_process;
	process_struc_ptr	parent;
	child_struc_ptr		first_child;
	thread_struc_ptr	first_thread;
	module_struc_ptr	first_module;
	pointer_data 		* module_list;
	mmap_struc_ptr		first_mmap;
	comm_struc_ptr		first_comm;
	module_struc_ptr	current_module;
	char*			name;
	int*			sample_count;
	int*			sample_order;
	int			module_count;
	uint32_t		pid;
	uint32_t		tid_main;
	int			cycle_count;
	int			inst_count;
	int			total_sample_count;
	}process_data;

typedef struct child_struc{
	child_struc_ptr		next;
	child_struc_ptr		previous;
	process_struc_ptr	this_process;
	process_struc_ptr	parent;
	} child_data;
	
typedef struct sample_struc{
	sample_struc_ptr	next;
        sample_struc_ptr	previous;
	uint64_t		rva;
        process_struc_ptr	this_process;
        thread_struc_ptr	this_thread;
        module_struc_ptr	this_module;
        function_struc_ptr	this_function;
	branch_struc_ptr	return_list;
	branch_struc_ptr	call_list;
	branch_struc_ptr	next_taken_list;
	char*			asm_string;
	int*			sample_count;
	int*			sample_order;
	float*			ratios;
	int*			ratio_order;
	uint64_t		ip;
	int			source_file_line;
	int			compilation_file_line;
	int			total_sample_count;
	int			total_sources;
	int			total_targets;
	int			total_taken_branch;
	}sample_data;
	
typedef struct branch_struc{
	branch_struc_ptr	next;
	branch_struc_ptr	previous;
	uint64_t		address;
	char*			module_name;
	char*			function_name;
	module_struc_ptr	this_module;
	function_struc_ptr	this_function;
	int			count;
	}branch_data;

typedef struct branch_site_struc{
	branch_site_struc_ptr	next;
	uint64_t		address;
	int			count;
	}branch_site_data;

typedef struct func_branch_struc{
	func_branch_struc_ptr	next;
	branch_struc_ptr	this_branch_target;
	branch_site_struc_ptr	branch_site_stack;
	}func_branch_struc_data;
	

typedef struct call_chain_struc{
	call_chain_struc_ptr	next;
	call_chain_struc_ptr	previous;
	call_chain_struc_ptr	stack;
	module_struc_ptr	this_module;
	function_struc_ptr	this_function;
	uint64_t		address;
	int			count;
	}call_chain_data;
	
typedef struct hash_struc{
	sample_struc_ptr 	this_sample;
	uint64_t 		this_rva;
	hash_struc_ptr		next;
	}hash_data;
	
typedef struct rva_hash_struc{
	int			size;
	int			entries;
	hash_data	 *	this_array;
	}rva_hash_data;

typedef struct function_location_stack{
	function_location_stack_ptr	next;
	function_location_stack_ptr	previous;
	char*		name;
	char*		bind;
	uint64_t	base;
	uint32_t	len;
	}function_loc_stack_data;

typedef struct function_location{
	function_struc_ptr	this_function;
	char*			name;
	char*			bind;
	uint64_t		base;
	uint32_t		len;
	}function_loc_data;

typedef struct functionlist_struc{
	function_loc_data *	list;
	int	size;
	}functionlist_data;	

typedef struct asm_struc{
	asm_struc_ptr		next;
	asm_struc_ptr		previous;
	uint64_t		address;
	uint64_t		target;
	branch_struc_ptr	return_list;
	branch_struc_ptr	call_list;
	branch_struc_ptr	next_taken_list;
	branch_site_struc_ptr	hottest_call_target;
	char *			asm_text;
	char *			encoding;
	char *			principal_source_file;
	char *			principal_source_name;
	char *			initial_source_file;
	char *			initial_source_name;
	int *			sample_count;
	int			principal_source_line;
	int			initial_source_line;
	int			branch;
	int			call;
	int			conditional;
	int			total_sample_count;
	}asm_data;
	
typedef struct basic_block_struc{
	basic_block_struc_ptr	next;
	basic_block_struc_ptr	previous;
	uint64_t	address;
	uint64_t	end_address;
	uint64_t	target1;
	uint64_t	target2;
	char *		text;
	char *		encoding;
	int *		sample_count;
	int		source_line;
	int		block_count;
	int		branch;
	int		call;
	int		total_sample_count;
	}basic_block_data;

typedef struct source_line_struc{
	source_line_struc_ptr	next;
	source_line_struc_ptr	previous;
	char*			source_text;
	int*			sample_count;
	int			line;
	}source_line_data;

typedef struct file_list_struc{
	file_list_struc_ptr	next;
	file_list_struc_ptr	previous;
	char*			principal_source_file;
	char*			principal_source_name;
	int			count;
	}file_list_data;

typedef struct addr_list_struc{
	uint64_t	base;
	uint64_t	len;
	void*		struc_ptr;
	}addr_list_data;

extern module_struc_ptr module_stack, current_module;
extern process_struc_ptr process_stack, active_proc_stack, current_process;
extern process_struc_ptr principal_process_stack;
extern int num_process, global_func_count, global_sample_count_in_func,global_branch_sample_count;
extern pointer_data * process_list;
extern function_struc_ptr global_func_stack;
extern int * global_sample_count, total_sample_count;
extern double *global_multiplex_correction, uop_issue_rate;
extern char *gooda_dir;
extern int sample_struc_count, asm_struc_count, basic_block_struc_count;

function_struc_ptr function_struc_create();
source_struc_ptr source_struc_create();
principal_file_struc_ptr principal_file_struc_create();
module_struc_ptr module_struc_create();
thread_struc_ptr thread_struc_create();
process_struc_ptr process_struc_create();
child_struc_ptr child_struc_create();
sample_struc_ptr sample_struc_create();
rva_hash_struc_ptr rva_hash_struc_create(int len);
hash_struc_ptr hash_struc_create();
functionlist_struc_ptr functionlist_struc_create(int len);
asm_struc_ptr asm_struc_create();
basic_block_struc_ptr basic_block_struc_create();
function_location_stack_ptr function_location_stack_create();
functionlist_struc_ptr get_functionlist(module_struc_ptr this_module);
pointer_data * sort_global_func_list();
file_list_struc_ptr file_list_create();
source_line_struc_ptr source_line_create();
branch_struc_ptr branch_struc_create();
func_branch_struc_ptr func_branch_struc_create();
branch_site_struc_ptr branch_site_struc_create();
