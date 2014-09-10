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
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <malloc.h>
#include "perf_event.h"
#include "gooda.h"
#include "perf_gooda.h"
#include "gooda_util.h"

thread_struc_ptr base_thread;
process_struc_ptr process_minus_one = NULL;
process_struc_ptr base_proc;
int util_dbg_flag =0;
char vmlinux[]="/vmlinux";
int max_print=20;
int print_rva=0;
uint64_t fourk_align=0xFFFFFFFFFFFFF000UL;

mmap_struc_ptr 
mmap_copy(mmap_struc_ptr mmap_orig, uint32_t new_pid, uint64_t new_time)
{
	mmap_struc_ptr new_struc;

	new_struc = mmap_struc_create();
	if(new_struc == NULL)
		{
		fprintf(stderr," failed to create new mmap in mmap_copy\n");
		err(1," failed to make new mmap in mmap_copy");
		}
	new_struc->pid = new_pid;
	new_struc->tid = new_pid;
	new_struc->addr = mmap_orig->addr;
	new_struc->len = mmap_orig->len;
	new_struc->pgoff = mmap_orig->pgoff;
	new_struc->is_kernel = mmap_orig->is_kernel;
	new_struc->filename = mmap_orig->filename;
	new_struc->time = new_time;
#ifdef DBUG
	fprintf(stderr," from mmap_copy, pid = %d, filename = %s\n",new_struc->pid,new_struc->filename);
	fprintf(stderr," address of mmap_orig = %p, orig_pid, %d, addr = 0x%"PRIx64", address of orig filename = %p, filename = %s\n",
		mmap_orig, mmap_orig->pid, mmap_orig->addr, &mmap_orig->filename, mmap_orig->filename);
//		fprintf(stderr,"kernel_mmap address = %lp, filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap,kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
	return new_struc;
}

void* 
init(void)
{
	uint32_t base_pid =0xFFFFFFFF, base_tid=0;
	char *base_name, *kern_name, *ret, name[] = "aggregated_kernel_object", kname[] = "vmlinux" ;
	int nlen;
	uint64_t tzero = 0;
	int last, i;
	
	sqrt_five = sqrt(5.0);


//	create comm structure for PID = -1
//		since perf does not

	nlen = strlen(name);

	base_name = (char*) malloc((nlen + 1)*sizeof(char));
	if(base_name == NULL)
		{
		printf("failed to malloc char string in init\n");
		err(1,"failed to malloc char in init");
		}
	ret = strcpy(base_name,name);
	if(ret != base_name)
		{
		printf(" problem with strcpy in inint\n");
		err(1," failed strcpy in init");
		}


	nlen = strlen(kname);
	kern_name = (char*) malloc((nlen+1)*sizeof(char));
	if(kern_name == NULL)
		{
		printf("failed to malloc char string in init, kern_name\n");
		err(1,"failed to malloc char in init");
		}
	ret = strcpy(kern_name,kname);
	if(ret != kern_name)
		{
		printf(" problem with strcpy in inint, kern_name\n");
		err(1," failed strcpy in init");
		}

//	create first process struc and initialize process_stack
	base_proc = process_struc_create();
	if(base_proc == NULL)
		{
		printf(" failed to create base proc struc in init\n");
		err(1, "failed to create base proc in inint");
		}
	num_process++;
	process_stack = base_proc;
	base_proc->pid = base_pid;
	base_proc->name = base_name;
	base_thread = thread_struc_create();
	if(base_thread == NULL)
		{
		printf(" failed to create base thread struc in init\n");
		err(1, "failed to create base thread in inint");
		}
	base_thread->tid = 0;
	base_proc->first_thread = base_thread;
	return;
}

void* 
dump_mmstack(mmap_struc_ptr top_of_stack)
{
	mmap_struc_ptr this_mmap;
	this_mmap = top_of_stack;
	while(this_mmap != NULL)
		{
		fprintf(stderr, " mmap path = %s, mmap_len = 0x%"PRIx64", mmap_pid = %d, mmap_addr = 0x%"PRIx64", time = 0x%"PRIx64"\n",
			this_mmap->filename, this_mmap->len, this_mmap->pid, this_mmap->addr, this_mmap->time);
		this_mmap = this_mmap->next;
		}
}

void* 
dump_process_stack(void)
{
	process_struc_ptr this_process, process_tmp;
	module_struc_ptr this_module;
	mmap_struc_ptr this_mmap;
	
	this_process = process_stack;
	while(this_process != NULL)
		{
		fprintf(stderr," process %s, pid = %d\n",this_process->name,this_process->pid);
		this_module = this_process->first_module;
		while(this_module!= NULL)
			{
			fprintf(stderr," module path = %s, length = 0x%"PRIx64"\n",this_module->path,this_module->length);
			this_module = this_module->next;
			}
		this_mmap = this_process->first_mmap;
		while(this_mmap != NULL)
			{
			fprintf(stderr," mmap path = %s, mmap_len = 0x%"PRIx64", mmap_pid = %d, mmap_addr = 0x%"PRIx64", time = 0x%"PRIx64"\n",
				this_mmap->filename, this_mmap->len, this_mmap->pid, this_mmap->addr, this_mmap->time);
			this_mmap = this_mmap->next;
			}
		this_process = this_process->next;
		}
	return;
}

void* 
dump_process(process_struc_ptr this_process)
{
	process_struc_ptr  process_tmp;
	module_struc_ptr this_module;
	mmap_struc_ptr this_mmap;

	fprintf(stderr," in dump_process\n");	
	if(this_process != NULL)
		{
		fprintf(stderr," process %s, pid = %d\n",this_process->name,this_process->pid);
		this_module = this_process->first_module;
		while(this_module!= NULL)
			{
			fprintf(stderr," module path = %s, length = 0x%"PRIx64"\n",this_module->path,this_module->length);
			this_module = this_module->next;
			}
		this_mmap = this_process->first_mmap;
		while(this_mmap != NULL)
			{
			fprintf(stderr," mmap path = %s, mmap_len = 0x%"PRIx64", mmap_pid = %d, mmap_addr = 0x%"PRIx64", time = 0x%"PRIx64", process_struc addr = %p, module_struc addr = %p\n",
				this_mmap->filename, this_mmap->len, this_mmap->pid, this_mmap->addr, this_mmap->time, this_mmap->this_process,this_mmap->this_module);
			fprintf(stderr," linked list addresses next = %p, previous = %p\n",this_mmap->next, this_mmap->previous);
			this_mmap = this_mmap->next;
			}
		}
	return;
}

mmap_struc_ptr 
initialize_mmap_stack(process_struc_ptr this_process,uint32_t pid,uint64_t time)
{
	mmap_struc_ptr this_mmap, loop_mmap;
	module_struc_ptr this_module, loop_module;

	loop_mmap = base_proc->first_mmap;
	while(loop_mmap != NULL)
		{
		this_mmap = mmap_copy(loop_mmap,pid, time);
		this_mmap->next = this_process->first_mmap;
		if(this_process->first_mmap != NULL) this_process->first_mmap->previous = this_mmap;
		this_process->first_mmap = this_mmap;
		this_mmap->this_process = this_process;

		loop_mmap = loop_mmap->next;
		}
	return this_process->first_mmap;
}

void* 
create_process_zero(uint64_t this_time)
{
//	function required because perf insists 
//	on not making a comm record for pid 0,  tid 0

	process_struc_ptr this_process, process_tmp;
	module_struc_ptr this_module;
	mmap_struc_ptr this_mmap;
	comm_struc_ptr fixup_comm_ptr;
	char  *filename, kname[] = "vmlinux" ;
	int i, func_len;
	
	func_len = strlen(kname);
	filename = (char*) malloc((func_len+2)*sizeof(char));
	for(i=0; i<func_len; i++) filename[i] = kname[i];
	filename[func_len+1] = '\0';
	
	this_process = process_stack;
#ifdef DDBUG
	dump_process_stack();
#endif
		fixup_comm_ptr = comm_struc_create();
		if(fixup_comm_ptr == NULL)
			{
			fprintf(stderr,"failed to create fixup_comm during create_process_zero, pid = 0\n");
			err(1,"failed to create fixup_comm during insert mmap");
			}
		fixup_comm_ptr->pid = 0;
		fixup_comm_ptr->tid = 0;
		fixup_comm_ptr->name = filename;
		fixup_comm_ptr->time = this_time;
#ifdef DBUG
		fprintf(stderr," insert mmap fixup for unknown PID = %d, tid = %d, filename = %s\n",fixup_comm_ptr->pid,fixup_comm_ptr->tid,fixup_comm_ptr->name);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
		this_process = insert_comm(fixup_comm_ptr);
	return;
}

void* 
insert_event_names(int nre, event_name_struc_ptr name_list)
{
	int i,j,k,len;
	int * found_config;

//#ifdef DBUG
	fprintf(stderr,"entering insert_event_names, nre = %d, num_events = %d\n",nre,num_events);
	if(nre == num_events)
		{
		for(i=0; i< num_events; i++)fprintf(stderr," item %d global config = 0x%"PRIx64",list_config = 0x%"PRIx64", name = %s, period = %ld\n",i,global_attrs[i].attr.config, name_list[i].config,name_list[i].name,global_attrs[i].attr.sample.sample_period);
		}
//#endif
	if(nre != num_events)
		{
		fprintf(stderr," number of event names = %d != number of events = %d\n",nre,num_events);
		err(1," number of event names does not equal number of events");
		}

	event_list = (event_name_struc_ptr) malloc(nre*sizeof(event_name_data));
	if(event_list == NULL)
		{
		fprintf(stderr,"failed to malloc event_list in insert_event_names\n");
		err(1,"malloc failed in insert_event_names");
		}
//	put event_list into the same order as global_a
	found_config = (int*)malloc(num_events*sizeof(int));
	if(found_config == NULL)
		{
		fprintf(stderr,"failed to malloc found_config list in insert_event_names\n");
		err(1,"malloc failed in insert_event_names");
		}
	for(i=0; i< num_events; i++)found_config[i] = 0;

	for(i = 0; i< num_events; i++)
		{
		for(j=0; j < num_events; j++)
			{
			if(found_config[i] == 1) break;
			if(global_attrs[i].attr.config == name_list[j].config)
				{
				found_config[i] = 1;
				len = strlen(name_list[j].name);
				event_list[i].name = (char*) malloc(len+1);
				if(event_list[i].name == NULL)
					{
					fprintf(stderr,"failed to malloc event name in insert_event_names\n");
					err(1,"failed to malloc space for event name");
					}
				for(k=0;k<len;k++)
					{
					if((name_list[j].name[k] == ':') && (name_list[j].name[k+1] == 'p') 
						&& (name_list[j].name[k+2] == 'e')
						)break;
					event_list[i].name[k] = name_list[j].name[k];
					}
				event_list[i].name[k] = '\0';
				event_list[i].config = name_list[j].config;
#ifdef DBUG
				fprintf(stderr," item %d, name = %s, config = 0x%"PRIx64"\n",i,event_list[i].name,event_list[i].config);
#endif
				}
			}
		}
//	initialize order array now that we know what events were actually collected
//	missing cycle tree/fixed analysis events will be mapped to the "null" event in sample_data[]
	init_order();

#ifdef DBUG
	fprintf(stderr,"returned from init_order\n");
#endif

	num_branch = global_event_order->num_branch;
	num_sub_branch = global_event_order->num_sub_branch;
//	create global_sample_count array

	global_sample_count = (int*) malloc((num_events*(num_cores + num_sockets +1) + num_branch + num_sub_branch + 1)*sizeof(int));
	if(global_sample_count == NULL)
		{
		fprintf(stderr, " failed to malloc array for global_sample_count\n");
		err(1," failed to malloc global_sample_count");
		}
	memset((void*) global_sample_count, 0, (size_t) (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1)*sizeof(int) );

//	create global_multiplex_correction array

	global_multiplex_correction = (double*) malloc((num_events*(num_cores + num_sockets +1) + num_branch + num_sub_branch + 1)*sizeof(double));
	if(global_multiplex_correction == NULL)
		{
		fprintf(stderr, " failed to malloc array for global_multiplex_correction\n");
		err(1," failed to malloc global_multiplex_correction");
		}
	for(i=0; i < (num_events*(num_cores+num_sockets+1) + num_branch + num_sub_branch + 1); i++)global_multiplex_correction[i] = 1.0;

// print out event name structure
	for(i = 0; i< num_events; i++)
		{
		fprintf(stderr," event_list[%d].name = %s, event_list[%d].config = 0x%"PRIx64"\n",i,event_list[i].name,i,event_list[i].config);
		}
}

void* 
insert_event_descriptions(int nr_attrs, int nr_ids, perf_file_attr_ptr attrs, event_id_ptr event_ids)
{
//	process the perf event structures so the FDs in the sample records can actually be processed

	int max_id=-1,i,j,len;
	num_events = nr_attrs;
	num_cores = nr_ids/num_events;
//	debug printout
	        fprintf(stderr,"nr_attrs=%d nr_ids=%d, num_cores = %d, num_sockets = %d, num_events = %d\n", 
			nr_attrs, nr_ids,num_cores,num_sockets,num_events);
        for (i=0; i < nr_ids; i++) 
		{
                fprintf(stderr,"event_id = %d, ID:%"PRIu64" cfg=0x%"PRIx64" sample_type=0x%"PRIx64"\n",
                        event_ids[i].attr_id,
			event_ids[i].id,
                        attrs[event_ids[i].attr_id].attr.config,
                        attrs[event_ids[i].attr_id].attr.sample_type);
        	}
	for(i=0;i<nr_ids;i++)
		{
		fprintf(stderr," event_ids[%d] = %ld, max_id = %d, min_event_id = %ld\n",
			i,event_ids[i].id,max_id,min_event_id);
		if((int)event_ids[i].id > max_id)max_id = (int)event_ids[i].id;
		if(event_ids[i].id < min_event_id)min_event_id = (int)event_ids[i].id;
		}
	len = max_id - min_event_id + 1;
	fprintf(stderr," id_array len = %d, max_id = %d, min_event_id = %ld\n",
		len,max_id,min_event_id);
	id_array = (int*) malloc(len*sizeof(int));
	for(i=0;i<nr_ids;i++)
		id_array[event_ids[i].id - min_event_id] = event_ids[i].attr_id;
	fprintf(stderr,"returning from insert_event_descriptions\n");
	return;
}

mmap_struc_ptr 
find_mmap(mmap_struc_ptr pid_mmap_stack, mm_struc_ptr this_mm, char* filename, uint64_t new_time)
{
        mmap_struc_ptr this_struc=NULL;

        mmap_current = pid_mmap_stack;
        while(mmap_current !=NULL)
                {
		if(mmap_current->len == this_mm->len)
			{
			if(strcmp(filename, mmap_current->filename) == 0) 
				{
                        	if(mmap_current->addr == this_mm->addr)
                                	{
					mmap_current->time = new_time;
					return mmap_current;
                                	}  // if mmap_current->addr
				} // if strcmp
                        }//if(mmap_current-len
                mmap_current = mmap_current->next;
                }
        return NULL;
}
	
thread_struc_ptr 
find_thread_struc(process_struc_ptr this_process, uint32_t tid)
{
	thread_struc_ptr this_thread, thread_tmp;

	thread_tmp = this_process->first_thread;
	while(thread_tmp !=NULL)
		{
		if(thread_tmp->tid == tid)
			{
			this_thread = thread_tmp;
//	pop this thread to the top of stack
			if(this_thread != this_process->first_thread)
				{
				this_thread->previous->next = this_thread->next;
				if(this_thread->next != NULL)
					this_thread->next->previous = this_thread->previous;
				this_process->first_thread->previous = this_thread;
				this_thread->next = this_process->first_thread;
				this_process->first_thread = this_thread;
				}
			return this_thread;
			}
		thread_tmp = thread_tmp->next;
		}
#ifdef DBUG
	fprintf(stderr, " could not find thread tid = %d in stack for process pid = %d, address of thread_tmp = %p\n",tid,this_process->pid, thread_tmp);
#endif
//	err(1, " could not find tid in stack");
	return thread_tmp;
}

process_struc_ptr 
find_process_struc(uint32_t pid)
{
	process_struc_ptr this_process, process_tmp;
#ifdef DBUG
	fprintf(stderr," entering find_process  pid = %d, process_stack_pid = %d\n",pid, process_stack->pid);
#endif
	process_tmp = process_stack;
	while(process_tmp !=NULL)
		{
#ifdef DBUG
		fprintf(stderr," process_tmp->pid = %d, need pid = %d\n",process_tmp->pid, pid);
#endif
		if(process_tmp->pid == pid)
			{
			this_process = process_tmp;
//	pop this process to the top of stack
			if(this_process != process_stack)
				{
#ifdef DBUG
	fprintf(stderr," change process_stack old_pid = %d, new_pid =%d\n",process_stack->pid,this_process->pid);
#endif
				this_process->previous->next = this_process->next;
				if(this_process->next != NULL)
					this_process->next->previous = this_process->previous;
				process_stack->previous = this_process;
				this_process->next = process_stack;
				this_process->previous = NULL;
				process_stack = this_process;
				}
#ifdef DBUG
	fprintf(stderr," found process pid = %d, struc address =%p\n",process_stack->pid,this_process);
#endif
			return this_process;
			}
//		fprintf(stderr," in find_process while, pid = %d\n",process_tmp->pid);
		process_tmp = process_tmp->next;
		}
#ifdef DBUG
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
	fprintf(stderr," could not find process in stack pid = %d, process_stack_pid = %d\n",pid, process_stack->pid);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
//	printf(" could not find process in stack pid = %X, process_stack_pid = %X\n",pid, process_stack->pid);
//	err(1, " could not find pid in stack");
	return process_tmp;
}

module_struc_ptr 
find_module_struc(process_struc_ptr this_process, mmap_struc_ptr this_mmap)
{
	module_struc_ptr this_module, module_tmp;

	this_module = NULL;
	if(this_process->first_module == NULL) return this_module;
	module_tmp = this_process->first_module;
	while(module_tmp != NULL)
		{
		if(module_tmp->length == this_mmap->len)
			{
			if(strcmp(module_tmp->path, this_mmap->filename) == 0)
				return module_tmp;
			}
		module_tmp = module_tmp->next;
		}
	return module_tmp;
}
uint64_t
find_load_addr(char* this_path)
{
	char *local_name;
	char local_bin_dir[] = "./binaries";
	char local_linux[] = "/vmlinux";
	int is_linux, linux_offset;
	int local_bin_len=10;
	int access_status;
	int i,j,k,len,module_len,ret_val;
	uint64_t load_addr;
	int fd, close_status;

#ifdef DBUG
	fprintf(stderr," entring find_load_addr for module %s\n",this_path);
#endif
	linux_offset = 0x0;
	bin_type = 0;
//      strip off module name and create ./binaries/module_name string
	module_len = strlen(this_path);
	i = module_len;
	while((this_path[i] != '/') && ( i >0) )i--;
	len = module_len - i;   // + 1 for \0, -1 to get rid of /
	local_name = (char*) malloc((len + local_bin_len + 1)*sizeof(char));
	if(local_name == NULL)
		err(1,"failed to malloc local_name buffer in find_load_addr");
	for(i=0; i<local_bin_len; i++)local_name[i] = local_bin_dir[i];
	local_name[local_bin_len] =  '\0';
	is_linux = 0;
	if(strcmp(local_name, local_linux) == 0)is_linux = 1;
#ifdef DBUG
	fprintf(stderr," find_load_addr local_name for module %s\n",local_name);
#endif
	for(i=0; i<len; i++)local_name[local_bin_len+i] = this_path[module_len - len + i];
	local_name[local_bin_len + len] = '\0';
#ifdef DBUG
	fprintf(stderr," find_load_addr local_name for module %s, len = %d, module_len = %d\n",local_name,len,module_len);
#endif
	access_status = access(local_name, R_OK);
	if(access_status == 0)
		{
#ifdef DBUG
		fprintf(stderr," find_load_addr found local_name %s access_status = 0\n",local_name);
#endif
		fd = open(local_name, O_RDONLY);
		if (fd == -1)
			err(1," failed to open %s after access(%s) returned valid",local_name,local_name);
		load_addr = parse_elf_header(fd);
		if(load_addr == -1)
			{
			fprintf(stderr," find_load_addr returned -1 will set to 0: local module %s had address %lx\n",this_path,load_addr);
			load_addr = 0;
			}
#ifdef DBUG
		fprintf(stderr," find_load_addr: local module %s had address %lx\n",local_name,load_addr);
#endif
		close_status = close(fd);
		if(close_status != 0)
			err(1,"find_load_address failed to close module %s, open with local path %s\n",this_path,local_name);
		return load_addr;
		}
//	use full path
	access_status = access(this_path, R_OK);
	if(access_status == 0)
		{
#ifdef DBUG
		fprintf(stderr," find_load_addr found full path %s\n",this_path);
#endif
		fd = open(this_path, O_RDONLY);
		if (fd == -1)
			err(1," failed to open %s after access(%s) returned valid",this_path,this_path);
		load_addr = parse_elf_header(fd);
		if(load_addr == -1)
			{
			fprintf(stderr," find_load_addr returned -1 will set to 0: full path for module %s had address %lx\n",this_path,load_addr);
			load_addr = 0;
			}
#ifdef DBUG
		fprintf(stderr," find_load_addr: full path for module %s had address %lx\n",this_path,load_addr);
#endif
		close_status = close(fd);
		if(close_status != 0)
			err(1,"find_load_address failed to close module %s\n",this_path);
		return load_addr;
		}
	return 0;
	
}

module_struc_ptr 
bind_mmap(mmap_struc_ptr this_mmap)
{
	process_struc_ptr this_process;
	module_struc_ptr this_module;

//	find module with same path as mmap in principal_process module stack
//	or add a module structure to the principal_process moduel stack
//	fprintf(stderr,"in bind mmap, debug_flag = %d, this_mmap->filename = %s\n", debug_flag, this_mmap->filename);
#ifdef DBUG
	fprintf(stderr,"in bind mmap\n");
#endif
	this_process = this_mmap->principal_process;
//	if(debug_flag == 1)
//		fprintf(stderr," bind_mmap PID = %d, principal_name = %s, address %p\n",
//			this_process->pid, this_process->name, this_process);
#ifdef DBUG
	fprintf(stderr," bind_mmap PID = %d, name = %s\n",this_process->pid, this_process->name);
#endif
	this_module = find_module_struc(this_process, this_mmap);
#ifdef DBUG
	fprintf(stderr," bind_mmap find_module = %p\n",this_module);
#endif
	if(this_module == NULL)
		{
//		if(debug_flag == 1)
//			fprintf(stderr," bind_mmap: this_module = NULL, this_process name = %s, addr = %p\n",this_process->name, this_process);
		this_module = module_struc_create();
		if(this_module == NULL)
			{
			printf(" failed to create module_struc\n");
       	                printf("MMAP: PID:%d TID:%d ADDR:0x%"PRIx64" LEN:0x%"PRIx64" PGOFF:0x%"PRIx64" FILE:%s\n",
               	        this_mmap->pid,
                       	this_mmap->tid,
                        this_mmap->addr,
       	                this_mmap->len,
               	        this_mmap->pgoff,
                       	this_mmap->filename);
                       	err(1, "cannot create module struc");
                       	}
		this_module->path = this_mmap->filename;
#ifdef DBUG
	fprintf(stderr," bind_mmap after create this module path = %s\n",this_module->path);
#endif
		this_module->length = this_mmap->len;
		this_module->starting_ip = find_load_addr(this_module->path);
		this_module->bin_type = bin_type;
		this_module->time = this_mmap->time;
		this_module->is_kernel = this_mmap->is_kernel;
#ifdef DBUG
	fprintf(stderr," kernel flag for %s = %d\n",this_module->path, this_module->is_kernel);
#endif
//	put it on the top of the process' module stack and return
#ifdef DBUG
		fprintf(stderr,"module path = %s, starting_ip = 0x%"PRIx64", mmap addr = 0x%"PRIx64", len = 0x%"PRIx64"\n",
			this_module->path,this_module->starting_ip,this_mmap->addr,this_module->length);
#endif
		if(this_process->first_module != NULL) this_process->first_module->previous = this_module;
		this_module->next = this_process->first_module;
		this_process->first_module = this_module;

		}
//	if(this_module->is_kernel == 1)
//		this_mmap->addr = this_module->starting_ip;
	this_mmap->this_module = this_module;
	return this_module;
}
	
mmap_struc_ptr 
insert_mmap(mm_struc_ptr this_mm, char* filename, uint64_t new_time)
{
	mmap_struc_ptr this_struc, mmap_tmp, pid_mmap_stack;
	module_struc_ptr this_module, module_tmp;
	process_struc_ptr this_process;
	comm_struc_ptr fixup_comm_ptr;
	char kernel[]="[kernel.kallsyms]_text";
	char kernel_new[]="[kernel.kallsyms]_stext";
	int kern_mmap;

	kern_mmap = 0;

	this_process = find_process_struc(this_mm->pid);
#ifdef DBUG
	fprintf(stderr," insert_mmap found process %d, address = 0x%p\n",this_mm->pid,&this_mm->pid);
#endif
//	check if this record is a continuation mmap
//	and increase the length if it is and then return
	if(previous_mmap != NULL)
		{
		if(previous_mmap->pid == this_mm->pid)
			{
			if(strcmp(previous_mmap->filename, filename) == 0)
				{
//		same module as on previous mmap record check if length of mmap_struc has already been fully extended
				if(previous_mmap->last_pgoff >= this_mm->pgoff)return previous_mmap;
				previous_mmap->len += this_mm->len;
				previous_mmap->last_pgoff = this_mm->pgoff;
//	comment this out as this module gets set when events appear
//				previous_mmap->this_module->length += this_mm->len;
				return previous_mmap;
				}
			}
		}
//	this_process = find_process_struc(this_mm->pid);
	if(this_process == NULL)
		{
#ifdef DBUG
		fprintf(stderr,"failed to find process during insert mmap, pid = %d, filename = %s\n",this_mm->pid, filename);
//		err(1,"failed to find process during insert mmap");
#endif
		fixup_comm_ptr = comm_struc_create();
		if(fixup_comm_ptr == NULL)
			{
			fprintf(stderr,"failed to create fixup_comm during insert mmap, pid = %d\n",this_mm->pid);
			err(1,"failed to create fixup_comm during insert mmap");
			}
		fixup_comm_ptr->pid = this_mm->pid;
		fixup_comm_ptr->tid = this_mm->tid;
		fixup_comm_ptr->name = filename;
		fixup_comm_ptr->time = new_time;
#ifdef DBUG
		fprintf(stderr," insert mmap fixup for unknown PID = %d, tid = %d, filename = %s\n",fixup_comm_ptr->pid,fixup_comm_ptr->tid,fixup_comm_ptr->name);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
		this_process = insert_comm(fixup_comm_ptr);
		if(this_process == NULL)
			{
			fprintf(stderr,"failed to insert fixup_comm during insert mmap, pid = %d\n",this_mm->pid);
			err(1,"failed to insert fixup_comm during insert mmap");
			}
		}
	pid_mmap_stack = this_process->first_mmap;
#ifdef DBUG
	if(pid_mmap_stack == NULL)fprintf(stderr,"pid_mmap_stack == NULL\n");
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif

//	update the mmap structure destined for the process stack
	this_module = NULL;
//	find the mmmap entry matching path, location and length
	this_struc = find_mmap(this_process->first_mmap, this_mm, filename, new_time);
//	if(this_struc != NULL)fprintf(stderr," found mmap pid =%d, %s\n",this_struc->pid, this_struc->filename);
//	new mmap
	if(this_struc == NULL)
		{
		this_struc = mmap_struc_create();
		if(this_struc == NULL)
			{
			fprintf(stderr," failed to create mmap _struc\n");
			fprintf(stderr,"ERROR: MMAP: PID:%d TID:%d ADDR:0x%"PRIx64" LEN:0x%"PRIx64" PGOFF:0x%"PRIx64" FILE:%s\n",
			this_mm->pid,
			this_mm->tid,
			this_mm->addr,
			this_mm->len,
			this_mm->pgoff,
			filename);
			err(1, "cannot create mmap struc");
			}
		this_struc->pid = this_mm->pid;
		this_struc->tid = this_mm->tid;
		this_struc->addr = this_mm->addr;
		this_struc->len = this_mm->len;
//	stupid fixup for [kernel.kallsyms]_stext
		if( (strcmp(filename,kernel) == 0) || (strcmp(filename,kernel_new) == 0))
			{
			kern_mmap = 1;
//		kernel address can apprently be non zero now
//			if(this_struc->addr == 0)
//				{
//         make sure pgoff is 4K aligned..this will not be the case if mmap is for "kernel_new"
//		second fixup for kernels that insist on non zero value of ADDR
				this_struc->addr = (this_mm->pgoff & fourk_align);
				this_struc->len = this_mm->len - this_mm->pgoff;
				this_struc->is_kernel = 1;
//				}
			base_kern_address = this_struc->addr;
#ifdef DBUG
			fprintf(stderr,"fixup for kernel_kallsysms, old len = 0x%"PRIx64", new len = 0x%"PRIx64", pgoff = 0x%"PRIx64", addr = 0x%"PRIx64", kernel flag = %d\n",
				this_mm->len, this_struc->len, this_mm->pgoff, this_struc->addr, this_struc->is_kernel);
#endif
			}
		this_struc->filename = filename;
		if( (kern_mmap == 1) || (strcmp(filename,kernel) == 0) || (strcmp(filename,kernel_new) == 0))
			this_struc->filename = vmlinux;
		this_struc->time = new_time;
		this_struc->this_process = this_process;
//		fprintf(stderr," created new mmap pid = %d\n",this_struc->pid);
//	update the process mmap stack with the new entry
		if(pid_mmap_stack != NULL)
			{
			pid_mmap_stack->previous = this_struc;
			this_struc->next = pid_mmap_stack;
			}
		else
			{
			this_process->first_mmap = this_struc;
#ifdef DBUG
			fprintf(stderr," pid_mmap_stack = NULL for pid %d, %s\n",this_struc->pid,this_struc->filename);
#endif
			}
		pid_mmap_stack = this_struc;
		this_process->first_mmap = pid_mmap_stack;
		this_struc->previous = NULL;
		}
//	mmap already known, time does not need to be updated
	else if(this_struc != pid_mmap_stack)
		{
#ifdef DBUG
		fprintf(stderr," pop to top of stack pid = %d, %s\n",this_struc->pid,this_struc->filename);
#endif
//	pop this_struc to the top of the stack if it is not already there
		if(this_struc->previous == NULL)
			{
			fprintf(stderr," this_struc previous = NULL pid = %d, %s\n",this_struc->pid,this_struc->filename);
//			dump_mmstack(pid_mmap_stack);
			}

		this_struc->previous->next = this_struc->next;
		if(this_struc->next != NULL)
			this_struc->next->previous = this_struc->previous;
#ifdef DBUG
		fprintf(stderr," splice list pid = %d, %s\n",this_struc->pid,this_struc->filename);
#endif
		pid_mmap_stack->previous = this_struc;
		this_struc->next = pid_mmap_stack;
#ifdef DBUG
		fprintf(stderr," splice top of stack pid = %d, %s\n",this_struc->pid,this_struc->filename);
#endif
		pid_mmap_stack = this_struc;
		this_process->first_mmap = pid_mmap_stack;
		this_struc->previous = NULL;
		}
#ifdef DBUG
	fprintf(stderr,"returning mmap struc for %s starting at 0x%"PRIx64", len = 0x%"PRIx64", with time 0x%"PRIx64"\n",this_struc->filename,this_struc->addr,this_struc->len,this_struc->time);
#endif			
	return this_struc;
}

mmap_struc_ptr 
bind_sample(uint32_t pid, uint64_t ip, uint64_t new_time)
{
	process_struc_ptr this_process;
	mmap_struc_ptr this_mmap, mmap_tmp, pid_mmap_stack;
	module_struc_ptr this_module;

	this_mmap = NULL;	
	if(global_flag1 == 0)
		{
//		dump_process_stack();
		global_flag1++;
		}
#ifdef DBUG
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
	this_process = process_stack;
	if(this_process->pid != pid)
		this_process = find_process_struc(pid);
#ifdef DBUG
		if(this_process != NULL)
			{
			fprintf(stderr," from bind bind sample find_process_struc returned  = %p, %s\np - 0x%"PRIx64"",
				this_process, this_process->name,ip);
//			dump_mmstack(this_process->first_mmap);
			}
		else
			{
			fprintf(stderr," from bind bind sample find_process_struc returned  = %p\n",this_process);
			}

#endif
	if(this_process == NULL)
		{
#ifdef DBUG
		fprintf(stderr," from bind bind sample failed to find process for pid = %d\n",pid);
//		err(1," failed to find process in bind_sample");
#endif
		return this_mmap;
		}
	pid_mmap_stack = this_process->first_mmap;
	this_mmap = pid_mmap_stack;
	while(this_mmap != NULL)
		{
		if( this_mmap->pid == pid)
		  {
#ifdef DBUG
		fprintf(stderr,"this_mmap != NULL, right pid, ip = 0x%"PRIx64", name = %s\n",this_mmap->addr, this_mmap->filename);
#endif
		  if(this_mmap->addr < ip)
		    {
#ifdef DBUG
		fprintf(stderr,"this_mmap != NULL, right pid, above start ip = 0x%"PRIx64", name = %s\n",this_mmap->addr, this_mmap->filename);
#endif
		    if(this_mmap->len > ip - this_mmap->addr)
		      {
#ifdef DBUG
		fprintf(stderr,"this_mmap != NULL, right pid, in addr range, ip = 0x%"PRIx64", name = %s\n",this_mmap->addr, this_mmap->filename);
		fprintf(stderr," ip in addr range for %s, new time = 0x%"PRIx64", this_mmap->time = 0x%"PRIx64"\n",this_mmap->filename,new_time,this_mmap->time);
#endif
		      if(this_mmap->time < new_time)
			{
//	found appropriate mmap record, pop it to the top of stack
//   note:   if the OS unloads a module and then reloads it within the period of one cores collection buffer filling
//		AND the old and new load addresses overlap we still get it right
//		BECAUSE: we revert to the old mmap record and push it up in the stack  
//		and when the new mmap record on this core comes along, it gets pushed to the top
//	there may still be a problem if the new addr exactly equals the old addr
/*             
			if(pid_mmap_stack != this_mmap)
				{
				this_mmap->previous->next = this_mmap->next;
				if(this_mmap->next != NULL)this_mmap->next->previous = this_mmap->previous;
				pid_mmap_stack->previous = this_mmap;
				this_mmap->previous = NULL;
				this_mmap->next = pid_mmap_stack;
				pid_mmap_stack = this_mmap;
				this_process->first_mmap = pid_mmap_stack;
				}
*/
#ifdef DBUG
		fprintf(stderr," from bind  this_mmap->filename = %s\n",this_mmap->filename);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
			return this_mmap;
			}
		      }
		    }
		  }
		this_mmap = this_mmap->next;
		}
//	this might be an error
//	err(1,"no found mmap");
	return NULL;
}

process_struc_ptr 
find_principal_process(mmap_struc_ptr this_mmap)
{
	process_struc_ptr this_process, principal_process, loop_process;

//	find process with same path in principal_process_stack
//	or add this_mmap->this_process to the principal_process_stack
	this_process = this_mmap->this_process;
/*
	fprintf(stderr,"find_principal_process: this_process name = %s, addr = %p\n",this_process->name,this_process);
	if(debug_flag == 1)
		fprintf(stderr,"find_principal_process: this_process name = %s, addr = %p\n",this_process->name,this_process);
*/
	if(principal_process_stack == NULL)
		{
#ifdef DBUG
		fprintf(stderr,"principal_process_stack == NULL, this_mmap->pid = %d\n",this_mmap->pid);
#endif
/*
	if(debug_flag == 1)
		fprintf(stderr,"principal_process_stack == NULL, this_mmap->pid = %d\n",this_mmap->pid);
*/
		principal_process_stack = this_process;
		this_mmap->principal_process = this_process;
#ifdef DBUG
	fprintf(stderr,"princ_stack was null now = %p,stack->previous = %p,stack->next = %p\n",principal_process_stack,principal_process_stack->principal_previous,principal_process_stack->principal_next);
#endif
		return this_process;
	}
	loop_process = principal_process_stack;
#ifdef DBUG
	fprintf(stderr,"princ_stack = %p,stack->previous = %p,stack->next = %p\n",principal_process_stack,principal_process_stack->principal_previous,principal_process_stack->principal_next);
#endif
	while(loop_process != NULL)
		{
/*
		if(debug_flag == 1)
			fprintf(stderr,"find_principal_process: loop_process name = %s, addr = %p\n",loop_process->name,loop_process);
*/
		if(strcmp(loop_process->name,this_process->name) == 0)
			{
#ifdef DBUG
		fprintf(stderr,"matched principal name, this_mmap->pid = %d, loop_process->pid = %d\n",this_mmap->pid,loop_process->pid);
		fprintf(stderr,"matched principal name, this_process-name = %s, loop_process->name = %s\n",this_process->name,loop_process->name);
#endif
/*
			if(debug_flag == 1)
				{
				fprintf(stderr,"matched principal name, this_mmap->pid = %d, loop_process->pid = %d\n",
					this_mmap->pid,loop_process->pid);
				fprintf(stderr,"matched principal name, this_process-name = %s, loop_process->name = %s\n",
					this_process->name,loop_process->name);
				}
*/
//			found process by name in principal process stack
			this_mmap->principal_process = loop_process;
			if(loop_process != principal_process_stack)
				{
//			pop loop_process to the top of the principal_process_stack
				principal_process_stack->principal_previous = loop_process;
				loop_process->principal_previous->principal_next = loop_process->principal_next;
				if(loop_process->principal_next != NULL)loop_process->principal_next->principal_previous = loop_process->principal_previous;
				loop_process->principal_next = principal_process_stack;
				loop_process->principal_previous = NULL;
				principal_process_stack = loop_process;
#ifdef DBUG
				fprintf(stderr,"loop = %p,loop->previous = %p,loop->next = %p\n",loop_process,loop_process->principal_previous,loop_process->principal_next);
#endif
				}
			return loop_process;
			}
#ifdef DBUG
		fprintf(stderr,"loop_process = %p, next = %p\n",loop_process,loop_process->principal_next);
#endif
		loop_process = loop_process->principal_next;
		}
//	process name not found, add this process to the principal_process_stack
#ifdef DBUG
		fprintf(stderr,"pid name not found, this_mmap->pid = %d, old principal_stack->pid = %d\n",
			this_mmap->pid,principal_process_stack->pid);
		fprintf(stderr,"no match on process name, this_process-name = %s, principal_stack->name = %s\n",
			this_process->name,principal_process_stack->name);
#endif
/*
	if(debug_flag == 1)
		{
		fprintf(stderr,"pid name not found, this_mmap->pid = %d, old principal_stack->pid = %d\n",
			this_mmap->pid,principal_process_stack->pid);
		fprintf(stderr,"no match on process name, this_process-name = %s, principal_stack->name = %s\n",
			this_process->name,principal_process_stack->name);
		}
*/
	this_process->principal_next = principal_process_stack;
	principal_process_stack->principal_previous = this_process;
	principal_process_stack = this_process;
	this_process->principal_process = this_process;
	this_mmap->principal_process = this_process;
	return this_process;
}


process_struc_ptr 
insert_comm(comm_struc_ptr local_comm)
{
	process_struc_ptr this_process;
	thread_struc_ptr this_thread;
	module_struc_ptr base_module;
	mmap_struc_ptr base_mmap;
	uint64_t time_zero;

	if(global_flag2 == 0)
		{
		global_flag2 = 1;
		create_process_zero(time_zero);
#ifdef DBUG
//		dump_process_stack();
		fprintf(stderr," base_thread->sample_count address = %p\n",base_thread->sample_count);
#endif
		}
	time_zero = 0;
#ifdef DBUG
	fprintf(stderr,"entering insert_comm, pid = %d\n",local_comm->pid);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
	fprintf(stderr,"entering insert_comm, pid = %d, name = %s\n",local_comm->pid, local_comm->name);
#endif
	this_process = NULL;
	if(process_stack != NULL)this_process = find_process_struc( local_comm->pid);
	if(this_process == NULL)
		{
//		new process created by exec
#ifdef DBUG
		fprintf(stderr,"insert comm create process object for pid = %d, tid = %d\n",local_comm->pid,local_comm->tid);
		fprintf(stderr,"insert comm create process object for pid = %d, tid = %d, name = %s\n",local_comm->pid,local_comm->tid,local_comm->name);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
		this_process = process_struc_create();
		if(this_process == NULL)
			{
			fprintf(stderr,"insert comm failed to create process object for pid = %d, tid = %d\n",local_comm->pid,local_comm->tid);
			err(1, "insert comm cannot create process struc");
			}
		num_process++;
		process_stack->previous = this_process;
		this_process->next = process_stack;
		process_stack = this_process;
		this_process->pid = local_comm->pid;
		this_process->tid_main = local_comm->tid;
#ifdef DBUG
		fprintf(stderr," change process_stack old_pid = %d, new_pid =%d\n",process_stack->pid,this_process->pid);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
//		initialize the mmap and module stack for the process
		this_process->first_mmap = initialize_mmap_stack(this_process,this_process->pid,this_time);
#ifdef DBUG
		fprintf(stderr," process pid = %d, first_mmap path = %s\n",this_process->pid, this_process->first_mmap->filename);
		fprintf(stderr," process pid = %d, local_comm filename = %s, first_mmap path = %s\n",this_process->pid, local_comm->name, this_process->first_mmap->filename);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
		}
//	if this was not a new process then it is just a process or thread rename, 
//	or the records were out of order and an mmap record showed up first
//	fprintf(stderr," existing this_process in  insert_comm, pid = %d, tid = %d, new name = %s, old name = %s\n",local_comm->pid, local_comm->tid, local_comm->name,this_process->name);
	if(local_comm->tid == this_process->tid_main)
		this_process->name = local_comm->name;
#ifdef DBUG
	fprintf(stderr," have this_process in  insert_comm, pid = %d, tid = %d, name = %s\n",
		local_comm->pid, local_comm->tid, local_comm->name);
	fprintf(stderr," process pid = %d, tid_main = %d, first_mmap path address = %p\n",
		this_process->pid, this_process->tid_main, this_process->first_mmap->filename);
	fprintf(stderr," process pid = %d, first_mmap path = %s\n",this_process->pid, this_process->first_mmap->filename);
	fprintf(stderr," process pid = %d, filename = %s, first_mmap path = %s\n",this_process->pid, this_process->name, this_process->first_mmap->filename);
//		fprintf(stderr,"kernel_mmap filename = %s, kernel_mmap filename address = %lp\n",kernel_mmap->filename,kernel_mmap->filename);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
	if(this_process->first_mmap == NULL)
		{
		fprintf(stderr," failed to create kernel mmap in insert_comm for pid = %d\n",local_comm->pid);
		err(1," failed to create copy of kernel_mmap in insert_comm");
		}
//	pop to top of stack
	if(process_stack != this_process)
		{
		process_stack->previous = this_process;
		this_process->next = process_stack;
//		fprintf(stderr," change process_stack old_pid = %d, new_pid =%d\n",process_stack->pid,this_process->pid);
		process_stack = this_process;
		}
	
//	create and add thread_struc to process_struc linked list
	this_thread = find_thread_struc(this_process,local_comm->tid);
#ifdef DBUG
	fprintf(stderr," insert_comm create thread process for pid = %d, tid = %d\n",local_comm->pid,local_comm->tid);
#endif
	if(this_thread !=NULL)return this_process;
	this_thread = thread_struc_create();
	if(this_thread == NULL)
		{
		fprintf(stderr," insert_comm failed to create thread process for pid = %d, tis = %d\n",local_comm->pid,local_comm->tid);
		err(1, "insert_comm cannot create thread struc");
		}
	this_thread->tid = local_comm->tid;
	if(this_process->first_thread !=NULL)
				this_process->first_thread->previous = this_thread;
	this_thread->next = this_process->first_thread;
	this_process->first_thread=this_thread;
	return this_process;
}

process_struc_ptr 
insert_fork(fork_struc_ptr f)
{
//     two cases: thread creation (pid = ppid) or process fork (pid !=ppid)
	process_struc_ptr this_process,old_process;
	thread_struc_ptr this_thread, old_thread;
	child_struc_ptr this_child;
	module_struc_ptr this_module, old_module, loop_module;
	mmap_struc_ptr old_mmap, old_mmap_tmp, this_mmap, loop_mmap;

//	find ppid
	old_process = find_process_struc(f->ppid);
	if(old_process == NULL)
		{
#ifdef DBUG
		fprintf(stderr,"could not find old process for fork event, ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
#endif
//		err(1, "insert_fork could not find ppid in process stack");
		return old_process;
		}
#ifdef DBUG
			fprintf(stderr,"found old process for fork event in process list ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
#endif

	if(f->pid != f->ppid)
//		new process
		{
		this_process = find_process_struc(f->pid);
		if(this_process != NULL)
			{
#ifdef DBUG
			fprintf(stderr,"found new process for fork event in process list ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
#endif
//			err(1, "insert_fork found new pid in process stack");
			return this_process;
			}
#ifdef DBUG
			fprintf(stderr,"did not find new process for fork event in process list ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
#endif
		this_process = process_struc_create();
		if(this_process == NULL)
			{
			fprintf(stderr,"insert_fork failed to create process object for pid = %d, tid = %d\n",f->pid,f->tid);
			err(1, "insert_fork cannot create process struc");
			}
#ifdef DBUG
			fprintf(stderr,"created new process for fork event in process list ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
#endif
		num_process++;
		process_stack->previous = this_process;
		this_process->next = process_stack;
		process_stack = this_process;
		this_child = child_struc_create();
		if(this_child == NULL)
			{
			fprintf(stderr,"insert_fork failed to create child object for pid = %d, tid = %d\n",f->pid,f->tid);
			err(1, "insert_fork cannot create child struc");
			}
		this_process->pid = f->pid;
		this_process->parent = old_process;
		this_process->name = old_process->name;
		this_child->parent = old_process;
		this_child->this_process = this_process;
		if(old_process->first_child == NULL)
			{
			old_process->first_child = this_child;
			}
		else
			{
			old_process->first_child->previous = this_child;
			this_child->next = old_process->first_child;
			old_process->first_child = this_child;
			}
#ifdef DBUG
			fprintf(stderr,"made child structures for fork event in process list ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
			fprintf(stderr," dump process for old process ppid = %d\n",f->ppid);
//			dump_process(old_process);
#endif
//		clone the mmap's in reverse stack order and 
//		create a new module struc and 
//		add to the module stack of the cloned process
		old_mmap = old_process->first_mmap;
		old_mmap_tmp = old_mmap;
		while(old_mmap !=NULL)
			{
			old_mmap_tmp = old_mmap;
			old_mmap = old_mmap->next;
			}

		loop_mmap = old_mmap_tmp;
		while(loop_mmap != NULL)
//			walk the loop backwards and create new mmaps
			{
			this_mmap = mmap_copy(loop_mmap, f->pid, loop_mmap->time);
			this_mmap->next = this_process->first_mmap;
			if(this_process->first_mmap != NULL) this_process->first_mmap->previous = this_mmap;
			this_process->first_mmap = this_mmap;
			this_mmap->this_process = this_process;
			loop_mmap = loop_mmap->previous;
			}
#ifdef DBUG
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
//		dump_process(this_process); 
#endif
		}
	else
//	new thread  same process
		{
		this_process = old_process;
		}
// for both cases create a new thread
#ifdef DBUG
			fprintf(stderr,"about to start on thread structures for fork event in process list ppid = %d, pid = %d, ptid = %d, tid = %d\n",f->ppid,f->pid,f->ptid,f->tid);
//		fprintf(stderr," base_thread->sample_count address = %lp\n",base_thread->sample_count);
#endif
	this_thread = thread_struc_create();
	if(this_thread == NULL)
		{
		fprintf(stderr,"insert_fork failed to create thread object for new pid = %d, tid = %d\n",f->pid,f->tid);
		err(1, "insert_fork cannot create thread struc for new pid");
		}
	this_thread->tid = f->tid;
	if(this_process->first_thread == NULL)
		{
		this_process->first_thread = this_thread;
		}
	else
		{
		this_thread->next = this_process->first_thread;
		this_process->first_thread->previous = this_thread;
		this_process->first_thread = this_thread;
		}
	return this_process;
}

hash_struc_ptr 
find_hash_entry(hash_struc_ptr this_hash, uint64_t rva, rva_hash_struc_ptr this_table, module_struc_ptr this_module)
{
	hash_struc_ptr this_hash_entry, tmp_entry;
	sample_struc_ptr this_sample;

	tmp_entry = this_hash;
	while(this_hash != NULL)
		{
		if(this_hash->this_rva == rva)
			{
			return this_hash;
			}
		tmp_entry = this_hash;
		this_hash = this_hash->next;
		}
	this_hash = hash_struc_create();
	if(this_hash == NULL)
		{
		fprintf(stderr," failed to create hash element in linked list finder for address 0x%"PRIx64"\n",rva);
		err(1, "failed to create hash entry in linked list");
		}
	this_table->entries++;
	tmp_entry->next = this_hash;
	this_hash->this_rva = rva;
	this_sample = sample_struc_create();
	if(this_sample == NULL)
		{
		fprintf(stderr," failed to create sample struc element in linked list finder for address 0x%"PRIx64"\n",rva);
		err(1, "failed to create sample entry in linked list");
		}
	this_hash->this_sample = this_sample;
	this_module->first_sample->previous = this_sample;
	this_sample->next = this_module->first_sample;
	this_sample->rva = rva;
	this_module->first_sample = this_sample;
#ifdef DBUG
//	follow a single address through if there are worries about lost samples/rva struc's
//	if(rva == 0x43e110)fprintf(stderr,"creating sample struc in find_hash_create for address 0x43e110\n");
#endif
	return this_hash;
}

rva_hash_struc_ptr 
create_big_table(module_struc_ptr this_module)
{
	int new_size, old_size, old_index, new_index;
	double val;
	uint64_t rva, this_rva, old_rva, len;
	rva_hash_struc_ptr new_table;
	hash_struc_ptr this_hash_entry, old_hash_entry, tmp_entry;
	hash_data *new_array, *old_array;
	sample_struc_ptr this_sample;

	fprintf(stderr,"creating larger table for module %s with entries %d\n",this_module->path,this_module->this_table->entries);
	len = this_module->length;
	old_size = this_module->this_table->size;
	old_array = this_module->this_table->this_array;
	new_size = 10*old_size;
	new_table = rva_hash_struc_create(new_size);
	if(new_table == NULL)
		{
		fprintf(stderr,"failed to create bigger hash table for module %s\n",this_module->path);
		err(1,"failed to extend hash table");
		}
	new_array = new_table->this_array;
#ifdef DBUG
	fprintf(stderr,"starting loop over old entries, old_size = %d, new size = %d, old_array address = %p\n",old_size,new_size,old_array);
#endif
	for(old_index = 0; old_index < old_size; old_index++)
		{
		rva = old_array[old_index].this_rva;
#ifdef DBUG
		fprintf(stderr,"rva = 0x%"PRIx64"\n",rva);
#endif
		if(rva != 0)
			{
			old_hash_entry = &old_array[old_index];
			while(old_hash_entry != NULL)
				{
				old_rva = old_hash_entry->this_rva;
				val = sqrt_five*(double)(old_rva & 0x7FFFFFFF);
				new_index = (int)(val) & 0x7FFFFFFF;
				new_index = (new_index & 0x7FFFFFFF)%new_size;
#ifdef DBUG
				fprintf(stderr," val = %g, old_rva = 0x%"PRIx64", index = %d\n",val, old_rva, new_index);
#endif

				this_hash_entry = &new_array[new_index];
				tmp_entry = this_hash_entry;
				if(this_hash_entry->next != NULL)
					{
					while(this_hash_entry !=NULL)
						{
						tmp_entry = this_hash_entry;
						this_hash_entry = this_hash_entry->next;
						}
					this_hash_entry = hash_struc_create();
					if(this_hash_entry == NULL)
						{
						fprintf(stderr," failed to create hash element in create_big_table for address 0x%"PRIx64"\n",old_rva);
						err(1, "failed to create hash entry in create_big_table");
						}
					tmp_entry->next = this_hash_entry;
					}
				this_hash_entry->this_rva = old_rva;
				this_hash_entry->this_sample = old_hash_entry->this_sample;
				new_table->entries++;
#ifdef DBUG
//	follow a single address through if there are worries about lost samples/rva struc's
//				if(old_rva == 0x43e110)fprintf(stderr," creating new hash in table expanding for address 0x43e110\n");
#endif
				old_hash_entry = old_hash_entry->next;
				}    //while old
			}    //if
		}	// for				
	this_module->this_table = new_table;
#ifdef DBUG
	fprintf(stderr,"finished making larger table\n");
#endif
	return new_table;
}
			

int
increment_module_struc(uint32_t pid, uint32_t tid, uint64_t ip, int this_event, int this_cpu, mmap_struc_ptr this_mmap, uint64_t time_enabled, uint64_t time_running)
{
	module_struc_ptr this_module, module_stack;
	process_struc_ptr this_process,principal_process;
	thread_struc_ptr this_thread, thread_stack;
	sample_struc_ptr this_sample;
	hash_struc_ptr this_hash_entry, this_hash; 
	hash_data *this_hash_array;

	uint64_t rva,tmp, four_hundredK = 0x400000, rva1;
	double val;
	int index;
	int offset, size;

	int sample_sum;
	sample_struc_ptr loop_sample;
#ifdef DBUG
	fprintf(stderr," in increment\n");
	fprintf(stderr," in increment module, pid = %d, tid = %d, ip = 0x%"PRIx64"\n",pid,tid,ip);
	fprintf(stderr,
	" in increment module structure, pid = %d, tid = %d, ip = 0x%"PRIx64", mmap base = 0x%"PRIx64", len = 0x%"PRIx64", total_samples = %d, this_cpu = %d, this event = %d\n",
			pid,tid,ip, this_mmap->addr, this_mmap->len, total_sample_count,this_cpu,this_event);
	fprintf(stderr," from this_mmap, this module address = %p, this_process address = %p\n",this_mmap->this_module, this_mmap->this_process);
#endif

	if(pid != pid_ker)
		{
		global_sample_count[num_cores*this_event + this_cpu ] += 1;
		global_sample_count[num_events*(num_cores + num_sockets) + this_event ] += 1;
		if((time_running > 0) && (time_running < time_enabled) ) 
			global_multiplex_correction[num_cores*this_event + this_cpu]  = (double)time_enabled/(double)time_running;
		}

	this_module = this_mmap->this_module;
	this_process = this_mmap->this_process;
	principal_process = this_mmap->principal_process;
	module_stack = principal_process->first_module;

	if(pid != pid_ker)
		{
		this_thread = find_thread_struc(this_process, tid);
		thread_stack = this_process->first_thread;
		}
#ifdef DBUG
	fprintf(stderr," process path = %s, module_path = %s\n", this_process->name, this_module->path);
#endif

	principal_process->total_sample_count++;
	principal_process->sample_count[num_cores*this_event + this_cpu ]= principal_process->sample_count[num_cores*this_event + this_cpu ] + 1;
	principal_process->sample_count[num_events*(num_cores + num_sockets) + this_event ]= principal_process->sample_count[num_events*(num_cores + num_sockets) + this_event  ] + 1;

#ifdef DBUG
	fprintf(stderr," incremented process total counts\n");
#endif
	this_module->total_sample_count++;
	this_module->sample_count[num_cores*this_event + this_cpu ]= this_module->sample_count[num_cores*this_event + this_cpu ] + 1;
	this_module->sample_count[num_events*(num_cores + num_sockets) + this_event ]= this_module->sample_count[num_events*(num_cores + num_sockets) + this_event  ] + 1;

#ifdef DBUG
	fprintf(stderr," incremented process, module total counts\n");
#endif
	if((this_thread !=NULL) && (pid != pid_ker))
		{
		this_thread->total_sample_count++;
//			this must be done here as the rva structures do not track the thread ID
//			the counts for process, module, function, etc can be reconstructed from the rva counts
#ifdef DBUG
		offset = num_cores*this_event + this_cpu;
		size = num_events*(num_cores+num_sockets+1);
		fprintf(stderr,"increment thread offset = %d, size = %d, sample_count_addr = %p, incremented address = %p, total_count address = %p\n",
		offset,size,this_thread->sample_count,&this_thread->sample_count[num_cores*this_event + this_cpu ], &this_thread->total_sample_count);
		fprintf(stderr," address of this_thread->sample_count[offset] = %p, value of this_thread->sample_count[offset] = %d, %x\n",
			&this_thread->sample_count[num_cores*this_event + this_cpu ], 
			this_thread->sample_count[num_cores*this_event + this_cpu ],  this_thread->sample_count[num_cores*this_event + this_cpu ]);
		if(offset > size)fprintf(stderr," ADDRESS ERROR in SAMPLE_COUNT\n");
#endif
		this_thread->sample_count[num_cores*this_event + this_cpu ]= this_thread->sample_count[num_cores*this_event + this_cpu ] + 1;
		}
#ifdef DBUG
	fprintf(stderr," incremented process, module and thread total counts\n");
		if((this_thread != NULL) && (pid != pid_ker))
			fprintf(stderr," address of this_thread->sample_count[offset] = %p, value of this_thread->sample_count[offset] = %d\n",
			&this_thread->sample_count[num_cores*this_event + this_cpu ], this_thread->sample_count[num_cores*this_event + this_cpu ]);
#endif

//	modified to use principal_process from this point down

//	pop to top of stack every pop_threshold samples
	if((principal_process->total_sample_count && pop_threshold) == 0)
		{
		if(principal_process_stack != principal_process)
			{
			principal_process->principal_previous->principal_next = principal_process->principal_next;
			if(principal_process->principal_next != NULL)principal_process->principal_next->principal_previous = principal_process->principal_previous;
			principal_process_stack->principal_previous = principal_process;
			principal_process->principal_next = principal_process_stack;
			principal_process_stack  = principal_process;
			}
		}
	if((this_module->total_sample_count && pop_threshold) == 0)
		{
		if(module_stack != this_module)
			{
			this_module->previous->next = this_module->next;
			if(this_module->next != NULL)this_module->next->previous = this_module->previous;
			module_stack->previous = this_module;
			this_module->next = module_stack;
			module_stack  = this_module;
			}
		}
	if((this_thread != NULL) && (pid != pid_ker))
	  {
	  if( (this_thread->total_sample_count && pop_threshold) == 0)
		{
		if(thread_stack != this_thread)
			{
			this_thread->previous->next = this_thread->next;
			if(this_thread->next != NULL)this_thread->next->previous = this_thread->previous;
			thread_stack->previous = this_thread;
			this_thread->next = thread_stack;
			thread_stack  = this_thread;
			}
		}

	  }
//	process address and increment per process/per module/per RVA/event_code/core_id count
#ifdef DBUG
	fprintf(stderr,"past stack popping\n");
#endif

	rva = ip - this_mmap->addr + this_module->starting_ip;
#ifdef DBUG
	fprintf(stderr,"RVA evaluation RVA = 0x%"PRIx64", IP = 0x%"PRIx64", mmap_addr = 0x%"PRIx64", elf_starting IP = 0x%"PRIx64"\n",
		rva, ip,  this_mmap->addr, this_module->starting_ip);
#endif
/*
	if(this_mmap->addr != four_hundredK)
		{
		rva1 = ip - this_mmap->addr;
		}
	else
		{
		rva1 = ip;
		}


	if((rva1 != rva) && (print_rva < max_print))
		{
		fprintf(stderr," ip = %lx, rva1 = %lx, rva = %lx, mmap_addr = %lx, starting_ip = %lx\n",
			ip,rva1,rva,this_mmap->addr, this_module->starting_ip);
		print_rva++;
		}
*/
	val = (double) (rva & 0x7FFFFFFF);

	if(this_module->this_table == NULL)
		this_module->this_table = rva_hash_struc_create(default_hash_length);
#ifdef DBUG
	fprintf(stderr," made hash table, len = %d\n", default_hash_length);
	fprintf(stderr," val = %f\n",val);
//	fprintf(stderr," sqrt_five = %f\n",sqrt_five);
#endif
	if(this_module->this_table == NULL)
		{
		fprintf(stderr, " increment module failed to create initial hash table, pid = %d, module = %s\n",pid,this_module->path);
		err(1, "increment module failed to create initial hash table");
		}

	val = val*sqrt_five;
	tmp = (uint64_t) val;
	index = (int) (tmp & 0x7FFFFFFF);
//	fprintf(stderr,"index = %x\n",index);
	index = index%this_module->this_table->size;
//	fprintf(stderr,"index = %x\n",index);
	this_hash_array = this_module->this_table->this_array;
	this_hash_entry = &this_hash_array[index];
#ifdef DBUG
	fprintf(stderr," got pointer to hash structure\n");
#endif

	if(this_hash_entry->this_rva != 0)
		{
//		fprintf(stderr," base entry exists\n");
//	base entry exists
//	find (or create) exact entry;
//	increment sample struc
		this_hash = this_hash_entry;
		if(this_hash_entry->this_rva != rva)
			{
#ifdef DBUG
			fprintf(stderr," hash rva != rva\n");
#endif
			this_hash = find_hash_entry(this_hash_entry, rva, this_module->this_table, this_module);
#ifdef DBUG
			fprintf(stderr," found hash entry != to hash table[index]\n");
#endif	
		}
#ifdef DBUG
		if(this_hash == NULL)fprintf(stderr," this_hash = NULL\n");
#endif
		this_hash->this_sample->sample_count[num_cores*this_event + this_cpu ]++;
		this_hash->this_sample->total_sample_count++;
		}
	else
		{
		
//	fill the empty array element
#ifdef DBUG
		fprintf(stderr," need to make new hash structure for this index\n");
#endif
		this_hash_entry->this_rva = rva;
		this_sample = sample_struc_create();
		if(this_sample == NULL)
			{
			fprintf(stderr," failed to create sample entry for module %s, rva = 0x%"PRIx64"\n",this_module->path,rva);
			err(1, "failed to create sample entry");
			}
#ifdef DBUG
		fprintf(stderr," first sample at this index\n");
#endif
		this_hash_entry->this_sample = this_sample;
		this_sample->sample_count[num_cores*this_event + this_cpu ]++;
		this_sample->total_sample_count++;
		this_sample->next = this_module->first_sample;
		this_sample->rva = rva;
		if(this_module->first_sample != NULL)
			this_module->first_sample->previous = this_sample;
		this_module->first_sample = this_sample;
		this_module->this_table->entries ++;
#ifdef DBUG
		fprintf(stderr," new sample, incremented table entries\n");
//	follow a single address through if there are worries about lost samples/rva struc's
//		if(rva == 0x43e110)fprintf(stderr," increment_module creating sample struc for 0x43e110\n");
#endif
		if(this_module->this_table->entries > this_module->this_table->size*max_entry_fraction)
//			create a new hash table and move all entries to the new table
			{
#ifdef DBUG
			fprintf(stderr,"making a bigger table for module = %s, process %d\n",this_module->path,pid);
#endif
			this_module->this_table = create_big_table(this_module);
			if(this_module->this_table == NULL)
				{
				fprintf(stderr," failed to create larger hash table for module %s\n",this_module->path);
				err(1, "failed to create larger hash");
				}
			}
		}
	return 0;
}

int
increment_return(mmap_struc_ptr this_mmap, uint64_t source, uint64_t destination, mmap_struc_ptr target_mmap)
{
	module_struc_ptr this_module, target_module;
	process_struc_ptr this_process,principal_process;
	sample_struc_ptr this_sample;
	hash_struc_ptr this_hash_entry, this_hash; 
	hash_data *this_hash_array;
	branch_struc_ptr this_return;

	uint64_t rva,target_rva,rva1,tmp, four_hundredK = 0x400000;
	double val;
	int index;
	int offset, size;

//	What is called a source or target depends on the usage.
//	this function is processing the LBRs associated with return instructions
//	and the rva is being defined by the source of the branch instruction
//	thus these returns are used to determine call counts from the instructions preceeding
//	the targets of these branches
//	BUT
//	for the display it is better to call the sample_count element at the call instruction the source
//	and the sample count in the function with the return, the target
//
#ifdef DBUG
	fprintf(stderr," entering increment_return, source = 0x%"PRIx64", destination = 0x%"PRIx64", module = %s\n",
			source,destination,this_mmap->this_module->path);
#endif

	if(util_dbg_flag == 0)
		{
		fprintf(stderr,"increment_target_site, target_index = %d\n",target_index);
		}
	this_module = this_mmap->this_module;
	this_module->total_sample_count++;
	this_module->sample_count[source_index]++;
	target_module = target_mmap->this_module;
	principal_process = this_mmap->principal_process;
	principal_process->total_sample_count++;
	principal_process->sample_count[source_index]++;
	global_sample_count[source_index]++;

	rva = source - this_mmap->addr + this_module->starting_ip;
	target_rva = destination - target_mmap->addr + target_module->starting_ip;
/*
	if(this_mmap->addr != four_hundredK)
		{
		rva1 = source - this_mmap->addr;
		}
	else
		{
		rva1 = source;
		}

	if((rva1 != rva) && (print_rva < max_print))
		{
		fprintf(stderr," source = %lx, rva1 = %lx, rva = %lx, mmap_addr = %lx, starting_ip = %lx\n",
			source,rva1,rva,this_mmap->addr, this_module->starting_ip);
		print_rva++;
		}
*/
	val = (double) (rva & 0x7FFFFFFF);

	if(this_module->this_table == NULL)
		this_module->this_table = rva_hash_struc_create(default_hash_length);
#ifdef DBUG
	fprintf(stderr," made hash table, len = %d\n", default_hash_length);
	fprintf(stderr," val = %f\n",val);
//	fprintf(stderr," sqrt_five = %f\n",sqrt_five);
#endif
	if(this_module->this_table == NULL)
		{
		fprintf(stderr, " increment module failed to create initial hash table, module = %s\n",this_module->path);
		err(1, "increment module failed to create initial hash table");
		}

	val = val*sqrt_five;
	tmp = (uint64_t) val;
	index = (int) (tmp & 0x7FFFFFFF);
//	fprintf(stderr,"index = %x\n",index);
	index = index%this_module->this_table->size;
//	fprintf(stderr,"index = %x\n",index);
	this_hash_array = this_module->this_table->this_array;
	this_hash_entry = &this_hash_array[index];
#ifdef DBUG
	fprintf(stderr," got pointer to hash structure \n");
	fprintf(stderr," got pointer to hash structure %p, hash rva = 0x%"PRIx64"\n",this_hash_entry, this_hash_entry->this_rva);
#endif

	if(this_hash_entry->this_rva != 0)
		{
//		fprintf(stderr," base entry exists\n");
//	base entry exists
//	find (or create) exact entry;
//	increment sample struc
		this_hash = this_hash_entry;
		if(this_hash_entry->this_rva != rva)
			{
#ifdef DBUG
			fprintf(stderr," hash rva != rva\n");
#endif
			this_hash = find_hash_entry(this_hash_entry, rva, this_module->this_table, this_module);
#ifdef DBUG
			fprintf(stderr," found hash entry != to hash table[index]\n");
#endif	
		}
#ifdef DBUG
		if(this_hash == NULL)fprintf(stderr," this_hash = NULL\n");
#endif

//		this_sample = this_hash_entry->this_sample;
		this_sample = this_hash->this_sample;

//this is where things chage with respect to increment_module..since we want to increment the linked list of return destinations
		this_sample->sample_count[source_index]++;
		if(this_sample->return_list == NULL)
			{
			this_sample->return_list = branch_struc_create();
			if(this_sample->return_list == NULL)
				err(1,"could not malloc buffer for top of rva struc return list");
			this_sample->return_list->address = target_rva;
			this_sample->return_list->this_module = target_module;
			this_sample->total_sources++;
			}
		this_return = this_sample->return_list;
		while(this_return->address != target_rva)
			{
			if(this_return->next == NULL)
				{
//			create new return struc and add it to the end of the stack
				this_return->next = branch_struc_create();
				if(this_return->next == NULL)
					err(1,"could not malloc buffer for next return list");
				this_return->next->previous = this_return;
				this_return->next->address = target_rva;
				this_return->next->this_module = target_module;
				this_sample->total_sources++;
				}
			this_return = this_return->next;
			}
		this_return->count++;
				

		this_hash->this_sample->total_sample_count++;
		global_branch_sample_count++;
		}
	else
		{
		
//	fill the empty array element
#ifdef DBUG
		fprintf(stderr," need to make new hash structure for this index\n");
#endif
		this_hash_entry->this_rva = rva;
		this_sample = sample_struc_create();
		if(this_sample == NULL)
			{
			fprintf(stderr," failed to create sample entry for module %s, rva = 0x%"PRIx64"\n",this_module->path,rva);
			err(1, "failed to create sample entry");
			}
#ifdef DBUG
		fprintf(stderr," first sample at this index\n");
#endif
		this_hash_entry->this_sample = this_sample;
//this is where things chage with respect to increment_module..since we want to increment the linked list of return destinations
		this_sample->return_list = branch_struc_create();
		if(this_sample->return_list == NULL)
			err(1,"could not malloc buffer for top of rva struc return list");
		this_sample->return_list->address = target_rva;
		this_sample->return_list->this_module = target_module;
		this_sample->return_list->count++;

		this_sample->sample_count[source_index]++;
		this_sample->total_sample_count++;
		this_sample->total_sources++;
		global_branch_sample_count++;
		this_sample->next = this_module->first_sample;
		this_sample->rva = rva;
		if(this_module->first_sample != NULL)
			this_module->first_sample->previous = this_sample;
		this_module->first_sample = this_sample;
		this_module->this_table->entries ++;
#ifdef DBUG
		fprintf(stderr," new sample, incremented table entries\n");
#endif
		if(this_module->this_table->entries > this_module->this_table->size*max_entry_fraction)
//			create a new hash table and move all entries to the new table
			{
#ifdef DBUG
			fprintf(stderr,"making a bigger table for module = %s\n",this_module->path);
#endif
			this_module->this_table = create_big_table(this_module);
			if(this_module->this_table == NULL)
				{
				fprintf(stderr," failed to create larger hash table for module %s\n",this_module->path);
				err(1, "failed to create larger hash");
				}
			}
		}
	return 0;

}

int
increment_call_site(mmap_struc_ptr source_mmap, uint64_t source, uint64_t destination, mmap_struc_ptr this_mmap)
{
	module_struc_ptr this_module, source_module;
	process_struc_ptr this_process,principal_process;
	sample_struc_ptr this_sample;
	hash_struc_ptr this_hash_entry, this_hash; 
	hash_data *this_hash_array;
	branch_struc_ptr this_call;

	uint64_t rva,source_rva,rva1,tmp, four_hundredK = 0x400000;
	double val;
	int index;
	int offset, size;

//	What is called a source or target depends on the usage.
//	this function is processing the LBRs associated with return instructions
//	and the rva is being defined by the target of the branch instruction
//	thus these returns are used to determine call counts from the instructions preceeding
//	the targets of these branches
//	BUT
//	for the display it is better to call the sample_count element at the call instruction the source
//	and the sample count in the function with the return, the target
//
//	this function reverses the branch direction of the returns placing the call count chain at the rva of the returns destination
//	this should be the the instruction after a call
//	ultimately the addresses in the linked list should be updated to the entry points of the function containing the return address
//	and maybe these events should be displayed at one asm instruction earlier
//	the function is called with this_mmap and target_mmap reversed with target mmap renamed source_mmap
//	this allows reuse (copy) of the code from increment_return

#ifdef DBUG
	fprintf(stderr," entering increment_call_site, source = 0x%"PRIx64", destination = 0x%"PRIx64", module = %s\n",
			source,destination,this_mmap->this_module->path);
#endif
	if(util_dbg_flag == 0)
		{
		fprintf(stderr,"increment_call_site, source_index = %d\n",source_index);
		util_dbg_flag = 1;
		}
	this_module = this_mmap->this_module;
	this_module->total_sample_count++;
	this_module->sample_count[target_index]++;
	source_module = source_mmap->this_module;
	principal_process = this_mmap->principal_process;
	principal_process->total_sample_count++;
	principal_process->sample_count[target_index]++;
	global_sample_count[target_index]++;

	rva = destination - this_mmap->addr + this_module->starting_ip;
	source_rva = source - source_mmap->addr + source_module->starting_ip;
/*
	if(this_mmap->addr != four_hundredK)
		{
		rva1 = destination - this_mmap->addr;
		}
	else
		{
		rva1 = destination;
		}

	if((rva1 != rva) && (print_rva < max_print))
		{
		fprintf(stderr," destination = %lx, rva1 = %lx, rva = %lx, mmap_addr = %lx, starting_ip = %lx\n",
			destination,rva1,rva,this_mmap->addr, this_module->starting_ip);
		print_rva++;
		}
*/
	val = (double) (rva & 0x7FFFFFFF);

	if(this_module->this_table == NULL)
		this_module->this_table = rva_hash_struc_create(default_hash_length);
#ifdef DBUG
	fprintf(stderr," made hash table, len = %d\n", default_hash_length);
	fprintf(stderr," val = %f\n",val);
//	fprintf(stderr," sqrt_five = %f\n",sqrt_five);
#endif
	if(this_module->this_table == NULL)
		{
		fprintf(stderr, " increment module failed to create initial hash table, module = %s\n",this_module->path);
		err(1, "increment module failed to create initial hash table");
		}

	val = val*sqrt_five;
	tmp = (uint64_t) val;
	index = (int) (tmp & 0x7FFFFFFF);
//	fprintf(stderr,"index = %x\n",index);
	index = index%this_module->this_table->size;
//	fprintf(stderr,"index = %x\n",index);
	this_hash_array = this_module->this_table->this_array;
	this_hash_entry = &this_hash_array[index];
#ifdef DBUG
	fprintf(stderr," got pointer to hash structure\n");
#endif

	if(this_hash_entry->this_rva != 0)
		{
//		fprintf(stderr," base entry exists\n");
//	base entry exists
//	find (or create) exact entry;
//	increment sample struc
		this_hash = this_hash_entry;
		if(this_hash_entry->this_rva != rva)
			{
#ifdef DBUG
			fprintf(stderr," hash rva != rva\n");
#endif
			this_hash = find_hash_entry(this_hash_entry, rva, this_module->this_table, this_module);
#ifdef DBUG
			fprintf(stderr," found hash entry != to hash table[index]\n");
#endif	
			}
#ifdef DBUG
		if(this_hash == NULL)fprintf(stderr," this_hash = NULL\n");
#endif
		this_sample = this_hash->this_sample;
//this is where things change with respect to increment_module..since we want to increment the linked list of return destinations
		this_sample->sample_count[target_index]++;
		if(this_sample->call_list == NULL)
			{
			this_sample->call_list = branch_struc_create();
			if(this_sample->call_list == NULL)
				err(1,"could not malloc buffer for top of rva struc call list");
			this_sample->call_list->address = source_rva;
			this_sample->call_list->this_module = source_module;
			this_sample->total_targets++;
			}
		this_call = this_sample->call_list;
		while(this_call->address != source_rva)
			{
			if(this_call->next == NULL)
				{
//			create new return struc and add it to the end of the stack
				this_call->next = branch_struc_create();
				if(this_call->next == NULL)
					err(1,"could not malloc buffer for next call list");
				this_call->next->previous = this_call;
				this_call->next->address = source_rva;
				this_call->next->this_module = source_module;
				this_sample->total_targets++;
				}
			this_call = this_call->next;
			}
		this_call->count++;
				

		this_hash->this_sample->total_sample_count++;
		global_branch_sample_count++;
		}
	else
		{
		
//	fill the empty array element
#ifdef DBUG
		fprintf(stderr," need to make new hash structure for this index\n");
#endif
		this_hash_entry->this_rva = rva;
		this_sample = sample_struc_create();
		if(this_sample == NULL)
			{
			fprintf(stderr," failed to create sample entry for module %s, rva = 0x%"PRIx64"\n",this_module->path,rva);
			err(1, "failed to create sample entry");
			}
#ifdef DBUG
		fprintf(stderr," first sample at this index\n");
#endif
		this_hash_entry->this_sample = this_sample;
//this is where things change with respect to increment_module..since we want to increment the linked list of return destinations
		this_sample->call_list = branch_struc_create();
		if(this_sample->call_list == NULL)
			err(1,"could not malloc buffer for top of rva struc call list");
		this_sample->call_list->address = source_rva;
		this_sample->call_list->this_module = source_module;
		this_sample->call_list->count++;
		this_sample->sample_count[target_index]++;
		this_sample->total_sample_count++;
		this_sample->total_targets++;
		global_branch_sample_count++;
		this_sample->next = this_module->first_sample;
		this_sample->rva = rva;
		if(this_module->first_sample != NULL)
			this_module->first_sample->previous = this_sample;
		this_module->first_sample = this_sample;
		this_module->this_table->entries++;
#ifdef DBUG
		fprintf(stderr," new sample, incremented table entries\n");
#endif
		if(this_module->this_table->entries > this_module->this_table->size*max_entry_fraction)
//			create a new hash table and move all entries to the new table
			{
#ifdef DBUG
			fprintf(stderr,"making a bigger table for module = %s\n",this_module->path);
#endif
			this_module->this_table = create_big_table(this_module);
			if(this_module->this_table == NULL)
				{
				fprintf(stderr," failed to create larger hash table for module %s\n",this_module->path);
				err(1, "failed to create larger hash");
				}
			}
		}
	return 0;

}

int
increment_next_taken_site(mmap_struc_ptr this_mmap, uint64_t source, uint64_t next_branch, mmap_struc_ptr next_taken_mmap)
{
	module_struc_ptr this_module, next_taken_module;
	process_struc_ptr this_process,principal_process;
	sample_struc_ptr this_sample;
	hash_struc_ptr this_hash_entry, this_hash; 
	hash_data *this_hash_array;
	branch_struc_ptr this_next_taken;

	uint64_t rva,next_taken_rva,rva1,tmp, four_hundredK = 0x400000;
	double val;
	int index;
	int offset, size;

//	What is called a source or target depends on the usage.
//	this function is processing the LBRs associated with return instructions
//	and the rva is being defined by the target of the branch instruction
//	thus these returns are used to determine call counts from the instructions preceeding
//	the targets of these branches
//	BUT
//	for the display it is better to call the sample_count element at the call instruction the source
//	and the sample count in the function with the return, the target
//
//	this function reverses the branch direction of the returns placing the call count chain at the rva of the returns destination
//	this should be the the instruction after a call
//	ultimately the addresses in the linked list should be updated to the entry points of the function containing the return address
//	and maybe these events should be displayed at one asm instruction earlier
//	the function is called with this_mmap and target_mmap reversed with target mmap renamed source_mmap
//	this allows reuse (copy) of the code from increment_return

#ifdef DBUG
	fprintf(stderr," entering increment_next_taken_site, source = 0x%"PRIx64", next_branch = 0x%"PRIx64", module = %s\n",
			source,next_branch,this_mmap->this_module->path);
#endif
	if(util_dbg_flag == 0)
		{
		fprintf(stderr,"increment_next_taken_site, next_taken_index = %d\n",next_taken_index);
		util_dbg_flag = 1;
		}
	this_module = this_mmap->this_module;
	this_module->total_sample_count++;
	this_module->sample_count[next_taken_index]++;
	next_taken_module = next_taken_mmap->this_module;
	principal_process = this_mmap->principal_process;
	principal_process->total_sample_count++;
	principal_process->sample_count[next_taken_index]++;
	global_sample_count[next_taken_index]++;

	if(this_mmap->addr != four_hundredK)
		{
		rva1 = source - this_mmap->addr;
		}
	else
		{
		rva1 = source;
		}
	rva = source - this_mmap->addr + this_module->starting_ip;
	next_taken_rva = next_branch - next_taken_mmap->addr + next_taken_module->starting_ip;

	if((rva1 != rva) && (print_rva < max_print))
		{
		fprintf(stderr," next_branch = %lx, rva1 = %lx, rva = %lx, mmap_addr = %lx, starting_ip = %lx\n",
			next_branch,rva1,rva,this_mmap->addr, this_module->starting_ip);
		print_rva++;
		}
	val = (double) (rva & 0x7FFFFFFF);

	if(this_module->this_table == NULL)
		this_module->this_table = rva_hash_struc_create(default_hash_length);
#ifdef DBUG
	fprintf(stderr," made hash table, len = %d\n", default_hash_length);
	fprintf(stderr," val = %f\n",val);
//	fprintf(stderr," sqrt_five = %f\n",sqrt_five);
#endif
	if(this_module->this_table == NULL)
		{
		fprintf(stderr, " increment module failed to create initial hash table, module = %s\n",this_module->path);
		err(1, "increment module failed to create initial hash table");
		}

	val = val*sqrt_five;
	tmp = (uint64_t) val;
	index = (int) (tmp & 0x7FFFFFFF);
//	fprintf(stderr,"index = %x\n",index);
	index = index%this_module->this_table->size;
//	fprintf(stderr,"index = %x\n",index);
	this_hash_array = this_module->this_table->this_array;
	this_hash_entry = &this_hash_array[index];
#ifdef DBUG
	fprintf(stderr," got pointer to hash structure\n");
#endif

	if(this_hash_entry->this_rva != 0)
		{
//		fprintf(stderr," base entry exists\n");
//	base entry exists
//	find (or create) exact entry;
//	increment sample struc
		this_hash = this_hash_entry;
		if(this_hash_entry->this_rva != rva)
			{
#ifdef DBUG
			fprintf(stderr," hash rva != rva\n");
#endif
			this_hash = find_hash_entry(this_hash_entry, rva, this_module->this_table, this_module);
#ifdef DBUG
			fprintf(stderr," found hash entry != to hash table[index]\n");
#endif	
			}
#ifdef DBUG
		if(this_hash == NULL)fprintf(stderr," this_hash = NULL\n");
#endif
		this_sample = this_hash->this_sample;
//this is where things change with respect to increment_module..since we want to increment the linked list of return destinations
		this_sample->sample_count[next_taken_index]++;
		if(this_sample->next_taken_list == NULL)
			{
			this_sample->next_taken_list = branch_struc_create();
			if(this_sample->next_taken_list == NULL)
				err(1,"could not malloc buffer for top of rva struc next_taken list");
			this_sample->next_taken_list->address = next_taken_rva;
			this_sample->next_taken_list->this_module = next_taken_module;
			this_sample->total_taken_branch++;
			}
		this_next_taken = this_sample->next_taken_list;
		while(this_next_taken->address != next_taken_rva)
			{
			if(this_next_taken->next == NULL)
				{
//			create new return struc and add it to the end of the stack
				this_next_taken->next = branch_struc_create();
				if(this_next_taken->next == NULL)
					err(1,"could not malloc buffer for next this_taken list");
				this_next_taken->next->previous = this_next_taken;
				this_next_taken->next->address = next_taken_rva;
				this_next_taken->next->this_module = next_taken_module;
				this_sample->total_taken_branch++;
				}
			this_next_taken = this_next_taken->next;
			}
		this_next_taken->count++;
				

		this_hash->this_sample->total_sample_count++;
		global_branch_sample_count++;
		}
	else
		{
		
//	fill the empty array element
#ifdef DBUG
		fprintf(stderr," need to make new hash structure for this index\n");
#endif
		this_hash_entry->this_rva = rva;
		this_sample = sample_struc_create();
		if(this_sample == NULL)
			{
			fprintf(stderr," failed to create sample entry for module %s, rva = 0x%"PRIx64"\n",this_module->path,rva);
			err(1, "failed to create sample entry");
			}
#ifdef DBUG
		fprintf(stderr," first sample at this index\n");
#endif
		this_hash_entry->this_sample = this_sample;
//this is where things change with respect to increment_module..since we want to increment the linked list of next taken branch locations
		this_sample->next_taken_list = branch_struc_create();
		if(this_sample->next_taken_list == NULL)
			err(1,"could not malloc buffer for top of rva struc next_taken list");
		this_sample->next_taken_list->address = next_taken_rva;
		this_sample->next_taken_list->this_module = next_taken_module;
		this_sample->next_taken_list->count++;
		this_sample->sample_count[next_taken_index]++;
		this_sample->total_sample_count++;
		this_sample->total_taken_branch++;
		global_branch_sample_count++;
		this_sample->next = this_module->first_sample;
		this_sample->rva = rva;
		if(this_module->first_sample != NULL)
			this_module->first_sample->previous = this_sample;
		this_module->first_sample = this_sample;
		this_module->this_table->entries++;
#ifdef DBUG
		fprintf(stderr," new sample, incremented table entries\n");
#endif
		if(this_module->this_table->entries > this_module->this_table->size*max_entry_fraction)
//			create a new hash table and move all entries to the new table
			{
#ifdef DBUG
			fprintf(stderr,"making a bigger table for module = %s\n",this_module->path);
#endif
			this_module->this_table = create_big_table(this_module);
			if(this_module->this_table == NULL)
				{
				fprintf(stderr," failed to create larger hash table for module %s\n",this_module->path);
				err(1, "failed to create larger hash");
				}
			}
		}
	return 0;

}


