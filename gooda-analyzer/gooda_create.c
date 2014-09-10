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

//	structure creation for Gooda
//     Generic Optimization Data Analyzer
//     Dispencer of wisdom and Insight
//     aka   DWI


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <malloc.h>
#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"
#include "gooda_util.h"

static inline int
get_count(void)
{
	return num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + num_derived + 1;
}

function_struc_ptr 
function_struc_create(void)
{

	function_struc_ptr this_struc;
/*
	this_struc = (function_struc_ptr) malloc(sizeof(function_data));
	if(this_struc == NULL)return this_struc;
	this_struc->next = NULL;
	this_struc->previous = NULL;
	this_struc->sources = NULL;
	this_struc->targets = NULL;
	this_struc->first_rva = NULL;
	this_struc->first_asm = NULL;
	this_struc->first_bb = NULL;
	this_struc->first_source_line = NULL;
	this_struc->function_name = NULL;
	this_struc->function_rva_start = 0;
	this_struc->function_length = 0;
	this_struc->source_file = NULL;
	this_struc->principal_file = NULL;
	this_struc->principal_file_list = NULL;
	this_struc->this_module = NULL;
	this_struc->this_process = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
	this_struc->sample_order = NULL;
	this_struc->cycle_count = 0;
	this_struc->inst_count = 0;
	this_struc->total_sample_count = 0;
	this_struc->total_branches = 0;
	this_struc->total_targets = 0;
	this_struc->total_sources = 0;
*/
	this_struc = calloc(1, sizeof(function_data));
	if(this_struc == NULL)return this_struc;
	this_struc->funclist_index = -1;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	
	return this_struc;
}

source_struc_ptr 
source_struc_create(void)
{
        source_struc_ptr this_struc;
/*
        this_struc = (source_struc_ptr) malloc(sizeof(source_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->source_file = NULL;
        this_struc->source_name = NULL;
        this_struc->path = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->sample_order = NULL;
        this_struc->cycle_count = 0;
        this_struc->inst_count = 0;
        this_struc->total_sample_count = 0;
*/
	this_struc = calloc(1, sizeof(source_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

        return this_struc;
}

principal_file_struc_ptr 
principal_file_struc_create(void)
{
        principal_file_struc_ptr this_struc;
/*
        this_struc = (principal_file_struc_ptr) malloc(sizeof(principal_file_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->principal_file_name = NULL;
        this_struc->path = NULL;
        this_struc->compile_options = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->sample_order = NULL;
        this_struc->cycle_count = 0;
        this_struc->inst_count = 0;
        this_struc->total_sample_count = 0;
*/
	this_struc = calloc(1, sizeof(principal_file_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

        return this_struc;
}

module_struc_ptr 
module_struc_create(void)
{
        module_struc_ptr this_struc;
/*
        this_struc = (module_struc_ptr) malloc(sizeof(module_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->this_table = NULL;
        this_struc->first_sample = NULL;
        this_struc->rva_list = NULL;
        this_struc->first_function = NULL;
        this_struc->current_function = NULL;
        this_struc->function_list = NULL;
        this_struc->first_source = NULL;
        this_struc->current_source = NULL;
        this_struc->module_name = NULL;
        this_struc->path = NULL;
        this_struc->local_path = NULL;
        this_struc->starting_ip = 0;
        this_struc->length = 0;
        this_struc->time = 0;
        this_struc->load_tsc = 0;
        this_struc->unload_tsc = 0;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->sample_order = NULL;
        this_struc->cycle_count = 0;
        this_struc->inst_count = 0;
        this_struc->total_sample_count = 0;
        this_struc->rva_count = 0;
        this_struc->total_branches = 0;
        this_struc->local_calls = 0;
        this_struc->remote_calls = 0;
*/
	this_struc = calloc(1, sizeof(module_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

        return this_struc;
}

thread_struc_ptr 
thread_struc_create(void)
{
        thread_struc_ptr this_struc;
/*
        this_struc = (thread_struc_ptr) malloc(sizeof(thread_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->sample_order = NULL;
	this_struc->tid = -1;
        this_struc->cycle_count = 0;
        this_struc->inst_count = 0;
        this_struc->total_sample_count = 0;
*/

	this_struc = calloc(1, sizeof(thread_data));
	if(this_struc == NULL)return this_struc;
	this_struc->tid = -1;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	
        return this_struc;
}

process_struc_ptr 
process_struc_create(void)
{
        process_struc_ptr this_struc;
/*
        this_struc = (process_struc_ptr) malloc(sizeof(process_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->principal_next = NULL;
        this_struc->principal_previous = NULL;
        this_struc->principal_process = NULL;
        this_struc->parent = NULL;
        this_struc->first_child = NULL;
        this_struc->first_thread = NULL;
        this_struc->first_module = NULL;
        this_struc->module_list = NULL;
        this_struc->first_mmap = NULL;
        this_struc->first_comm = NULL;
        this_struc->current_module = NULL;
        this_struc->name = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->sample_order = NULL;
        this_struc->module_count = 0;
        this_struc->pid = 0;
        this_struc->cycle_count = 0;
        this_struc->inst_count = 0;
        this_struc->total_sample_count = 0;
*/
	this_struc = calloc(1, sizeof(process_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

        return this_struc;
}

child_struc_ptr 
child_struc_create(void)
{
        child_struc_ptr this_struc;
/*
        this_struc = (child_struc_ptr) malloc(sizeof(child_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->this_process = NULL;
        this_struc->parent = NULL;
*/
	this_struc = calloc(1, sizeof(child_data));
        return this_struc;
}

sample_struc_ptr 
sample_struc_create(void)
{
        sample_struc_ptr this_struc;
/*
        this_struc = (sample_struc_ptr) malloc(sizeof(sample_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
	this_struc->rva = 0;
        this_struc->this_process = NULL;
        this_struc->this_thread = NULL;
        this_struc->this_module = NULL;
        this_struc->this_function = NULL;
        this_struc->asm_string = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->sample_order = NULL;
        this_struc->ratios = NULL;
        this_struc->ratio_order = NULL;
        this_struc->ip = 0;
        this_struc->source_file_line = 0;
        this_struc->compilation_file_line = 0;
        this_struc->total_sample_count = 0;
*/
	this_struc = calloc(1, sizeof(sample_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	
	if(this_struc != NULL)sample_struc_count++;
        return this_struc;
}


branch_struc_ptr
branch_struc_create(void)
{
	branch_struc_ptr this_struc;
	this_struc = calloc(1,sizeof(branch_data));
	return this_struc;
}

func_branch_struc_ptr
func_branch_struc_create(void)
{
	func_branch_struc_ptr this_struc;
	this_struc = calloc(1,sizeof(func_branch_struc_data));
	if(this_struc == NULL)return;
	this_struc->this_branch_target = calloc(1,sizeof(branch_data));
	if(this_struc->this_branch_target == NULL)
		{
		free(this_struc);
		return NULL;
		}
	return this_struc;
}

branch_site_struc_ptr
branch_site_struc_create(void)
{
	branch_site_struc_ptr this_struc;
	this_struc = calloc(1,sizeof(branch_site_data));
	return this_struc;
}

rva_hash_struc_ptr 
rva_hash_struc_create(int len)
{
	rva_hash_struc_ptr this_struc;
/*
	this_struc = (rva_hash_struc_ptr) malloc(sizeof(rva_hash_data));
        if(this_struc == NULL)return this_struc;
	this_struc->size = len;
	this_struc->entries = 0;
	this_struc->this_array = (hash_data *)malloc(len*sizeof(hash_data));
	if(this_struc->this_array == NULL)return NULL;
	memset((void *)this_struc->this_array, 0, (size_t)(len*sizeof(hash_data)) );
*/
	this_struc = calloc(1, sizeof(rva_hash_data));
	if(this_struc == NULL)return this_struc;
	this_struc->size = len;
	this_struc->this_array = calloc(len, sizeof(hash_data));
	if(this_struc->this_array == NULL)
		{
		free(this_struc);
		return NULL;
		}	

	return this_struc;
}

hash_struc_ptr 
hash_struc_create(void)
{
	hash_struc_ptr this_struc;
/*
	this_struc = (hash_struc_ptr) malloc(sizeof(hash_data));
        if(this_struc == NULL)return this_struc;
	this_struc->this_sample = NULL;
	this_struc->this_rva = 0;
	this_struc->next = NULL;
*/
	this_struc = calloc(1, sizeof(hash_data));
	return this_struc;
}

functionlist_struc_ptr 
functionlist_struc_create(int len)
{
	functionlist_struc_ptr this_struc;
/*
	this_struc = (functionlist_struc_ptr) malloc(sizeof(functionlist_data));
        if(this_struc == NULL)return this_struc;
	this_struc->size = len;
	this_struc->list = (function_loc_data *) malloc(len*sizeof(function_loc_data));
	if(this_struc->list == NULL)return NULL;
	memset((void*)this_struc->list, 0, (size_t)(len*sizeof(function_loc_data)));
*/
	this_struc = calloc(1, sizeof(rva_hash_data));
	if(this_struc == NULL)return this_struc;
	this_struc->size = len;
	this_struc->list = calloc(len, sizeof(function_loc_data));
	if(this_struc->list == NULL)
		{
		free(this_struc);
		return NULL;
		}	
	return this_struc;
}

function_location_stack_ptr 
function_location_stack_create(void)
{
	function_location_stack_ptr this_struc;
/*
	this_struc = (function_location_stack_ptr) malloc(sizeof(function_loc_stack_data));
        if(this_struc == NULL)return this_struc;
	this_struc->next = NULL;
	this_struc->previous = NULL;
	this_struc->base = 0;
	this_struc->len = 0;
	this_struc->name = NULL;
	this_struc->bind = NULL;
*/
	this_struc = calloc(1, sizeof(function_loc_stack_data));
	return this_struc;
}
asm_struc_ptr 
asm_struc_create(void)
{
        asm_struc_ptr this_struc;
/*
        this_struc = (asm_struc_ptr) malloc(sizeof(asm_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->asm_text = NULL;
        this_struc->encoding = NULL;
        this_struc->principal_source_file = NULL;
        this_struc->principal_source_name = NULL;
        this_struc->initial_source_file = NULL;
        this_struc->initial_source_name = NULL;
	this_struc->address = 0;
        this_struc->principal_source_line = 0;
        this_struc->initial_source_line = 0;
        this_struc->target = 0;
        this_struc->branch = 0;
        this_struc->call = 0;
	this_struc->total_sample_count = 0;
*/
	this_struc = calloc(1, sizeof(asm_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

	if(this_struc != NULL)asm_struc_count++;
        return this_struc;
}

basic_block_struc_ptr 
basic_block_struc_create(void)
{
        basic_block_struc_ptr this_struc;
/*
        this_struc = (basic_block_struc_ptr) malloc(sizeof(basic_block_data));
        if(this_struc == NULL)return this_struc;
        this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->text = NULL;
        this_struc->encoding = NULL;
	this_struc->address = 0;
	this_struc->end_address = 0;
	this_struc->target1 = 0;
	this_struc->target2 = 0;
        this_struc->source_line = 0;
        this_struc->block_count = 0;
        this_struc->branch = 0;
	this_struc->call = 0;
	this_struc->total_sample_count=0;
*/
	this_struc = calloc(1, sizeof(basic_block_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

	if(this_struc != NULL)basic_block_struc_count++;
        return this_struc;
}

source_line_struc_ptr 
source_line_create(void)
{
	source_line_struc_ptr this_struc;
/*
	this_struc = (source_line_struc_ptr)malloc(sizeof(source_line_data));
	if(this_struc == NULL)return this_struc;
	this_struc->next = NULL;
        this_struc->previous = NULL;
        this_struc->sample_count = (int*)malloc((num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(this_struc->sample_count == NULL)return NULL;
	memset((void*) this_struc->sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );
        this_struc->source_text = NULL;
	this_struc->line = 0;
*/
	this_struc = calloc(1, sizeof(source_line_data));
	if(this_struc == NULL)return this_struc;
	this_struc->sample_count = calloc(get_count(), sizeof(int));
	if(this_struc->sample_count == NULL)
		{
		free(this_struc);
		return NULL;
		}	

	return this_struc;
}

file_list_struc_ptr 
file_list_create(void)
{
	file_list_struc_ptr this_struc;
/*
	this_struc = (file_list_struc_ptr)malloc(sizeof(file_list_data));
	if(this_struc == NULL)return this_struc;
	this_struc->next = NULL;
	this_struc->previous = NULL;
	this_struc->principal_source_file = NULL;
	this_struc->principal_source_name = NULL;
	this_struc->count = 0;
*/
	this_struc = calloc(1, sizeof(source_line_data));
	return this_struc;
}

addr_list_struc_ptr 
addr_list_create(void)
{
	addr_list_struc_ptr this_struc;
/*
	this_struc = (addr_list_struc_ptr)malloc(sizeof(addr_list_data));
	this_struc->base = 0;
	this_struc->len = 0;
	this_struc->struc_ptr = NULL;
*/
	this_struc = calloc(1, sizeof(source_line_data));
	return this_struc;
}

